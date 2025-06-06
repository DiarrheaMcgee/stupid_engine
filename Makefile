.DEFAULT_GOAL := debug
NASM ?= nasm
CC ?= cc
NASMFLAGS = -felf64 -gdwarf
LIBS = -lvulkan -lXrandr -lX11
SUFFIXES += .d
INCLUDE = -I./dependencies/fast_obj -I./dependencies/stb -I./dependencies/RGFW -I./stupid
DEFAULT_CFLAGS = -MMD -D_POSIX_C_SOURCE=200809L -fPIC -ggdb3 -O3 -ansi -std=c11 -Wall -Werror -mavx -mavx2 -msse -msse2 -msse4.1 -mtune=znver3
VPATH = stupid/core:stupid/memory:stupid/renderer/vulkan:stupid/renderer:test
CSRC = \
	stupid/core/engine.c\
	stupid/core/logger.c\
	stupid/core/asserts.c\
	stupid/core/window.c\
	stupid/core/event.c\
	stupid/core/thread.c\
	stupid/memory/memory.c\
	stupid/renderer/vulkan/vulkan_backend.c\
	stupid/renderer/vulkan/vulkan_device.c\
	stupid/renderer/vulkan/vulkan_swapchain.c\
	stupid/renderer/vulkan/vulkan_image.c\
	stupid/renderer/vulkan/vulkan_command_buffer.c\
	stupid/renderer/vulkan/vulkan_fence.c\
	stupid/renderer/vulkan/vulkan_utils.c\
	stupid/renderer/vulkan/vulkan_memory.c\
	stupid/renderer/vulkan/vulkan_frontend.c\
	stupid/renderer/vulkan/vulkan_pipeline.c\
	stupid/renderer/render.c
	#xdg/*.c

ASMSRC = \
	stupid/memory/memcpy.asm\
	stupid/memory/memset.asm\
	stupid/memory/memeq.asm

BUILDDIR=out

$(BUILDDIR)/engine.o: stupid/core/engine.c
$(BUILDDIR)/logger.o: stupid/core/logger.c
$(BUILDDIR)/asserts.o: stupid/core/asserts.c
$(BUILDDIR)/window.o: stupid/core/window.c
$(BUILDDIR)/event.o: stupid/core/event.c
$(BUILDDIR)/thread.o: stupid/core/thread.c
$(BUILDDIR)/memory.o: stupid/memory/memory.c
$(BUILDDIR)/vulkan_backend.o: stupid/renderer/vulkan/vulkan_backend.c
$(BUILDDIR)/vulkan_device.o: stupid/renderer/vulkan/vulkan_device.c
$(BUILDDIR)/vulkan_swapchain.o: stupid/renderer/vulkan/vulkan_swapchain.c
$(BUILDDIR)/vulkan_image.o: stupid/renderer/vulkan/vulkan_image.c
$(BUILDDIR)/vulkan_command_buffer.o: stupid/renderer/vulkan/vulkan_command_buffer.c
$(BUILDDIR)/vulkan_fence.o: stupid/renderer/vulkan/vulkan_fence.c
$(BUILDDIR)/vulkan_utils.o: stupid/renderer/vulkan/vulkan_utils.c
$(BUILDDIR)/vulkan_memory.o: stupid/renderer/vulkan/vulkan_memory.c
$(BUILDDIR)/vulkan_frontend.o: stupid/renderer/vulkan/vulkan_frontend.c
$(BUILDDIR)/vulkan_pipeline.o: stupid/renderer/vulkan/vulkan_pipeline.c
$(BUILDDIR)/render.o: stupid/renderer/render.c
$(BUILDDIR)/memcpy.o: stupid/memory/memcpy.asm
$(BUILDDIR)/memset.o: stupid/memory/memset.asm
$(BUILDDIR)/memeq.o: stupid/memory/memeq.asm

COBJ = $(addprefix $(BUILDDIR)/,$(notdir $(CSRC:.c=.o)))
ASMOBJ = $(addprefix $(BUILDDIR)/,$(notdir $(ASMSRC:.asm=.o)))

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(INCLUDE) $(DEFAULT_CFLAGS) $(CFLAGS) -c -o $@ $<
-include $(COBJ:%.o=%.d)

$(BUILDDIR)/%.o: %.asm | $(BUILDDIR)
	$(NASM) $(NASMFLAGS) -o $@ $<

$(BUILDDIR)/%.so: %.o | $(BUILDDIR)
	$(CC) $(DEFAULT_CFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS) -shared -o $@ $^

$(BUILDDIR)/%.a: %.o | $(BUILDDIR)
	ar rcsu $@ $^

$(BUILDDIR)/libstupid.a: $(COBJ) $(ASMOBJ) | $(BUILDDIR)
	ar rcsu $@ $^

$(BUILDDIR)/stupid_test: test/main.c out/libstupid.a | $(BUILDDIR)
	$(CC) $(INCLUDE) $(DEFAULT_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: debug
debug: CFLAGS += -D_DEBUG
debug: $(BUILDDIR)/stupid_test
	./shaders.sh

.PHONY: release
release: $(BUILDDIR)/stupid_test
	./shaders.sh

.PHONY: clean
clean:
	rm -rf $(BUILDDIR)

