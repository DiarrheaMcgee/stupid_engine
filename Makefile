.DEFAULT_GOAL := debug
NASM ?= nasm
CC ?= cc
NASMFLAGS = -felf64 -gdwarf
LIBS = -lvulkan -lXrandr -lX11
SUFFIXES += .d
INCLUDE = -I./dependencies/fast_obj -I./dependencies/RGFW -I./include
DEFAULT_CFLAGS = -MMD -D_POSIX_C_SOURCE=200809L -fPIC -ggdb3 -O3 -ansi -std=c11 -Wall -Werror -mavx -mavx2 -msse -msse2 -msse4.1
VPATH = src:src/asm:src/render:src/render/vulkan:test
CSRC = \
	src/core/engine.c\
	src/core/logger.c\
	src/core/asserts.c\
	src/core/window.c\
	src/core/event.c\
	src/core/thread.c\
	src/memory/memory.c\
	src/render/vulkan/vulkan_backend.c\
	src/render/vulkan/vulkan_device.c\
	src/render/vulkan/vulkan_swapchain.c\
	src/render/vulkan/vulkan_image.c\
	src/render/vulkan/vulkan_command_buffer.c\
	src/render/vulkan/vulkan_fence.c\
	src/render/vulkan/vulkan_utils.c\
	src/render/vulkan/vulkan_memory.c\
	src/render/vulkan/vulkan_frontend.c\
	src/render/vulkan/vulkan_pipeline.c\
	src/render/render.c

ASMSRC = \
	src/asm/memcpy.asm\
	src/asm/memset.asm\
	src/asm/memeq.asm

BUILDDIR=out

$(BUILDDIR)/engine.o: src/engine.c
$(BUILDDIR)/logger.o: src/logger.c
$(BUILDDIR)/asserts.o: src/asserts.c
$(BUILDDIR)/window.o: src/window.c
$(BUILDDIR)/event.o: src/event.c
$(BUILDDIR)/thread.o: src/thread.c
$(BUILDDIR)/memory.o: src/memory.c
$(BUILDDIR)/vulkan_backend.o: src/render/vulkan/vulkan_backend.c
$(BUILDDIR)/vulkan_device.o: src/render/vulkan/vulkan_device.c
$(BUILDDIR)/vulkan_swapchain.o: src/render/vulkan/vulkan_swapchain.c
$(BUILDDIR)/vulkan_image.o: src/render/vulkan/vulkan_image.c
$(BUILDDIR)/vulkan_command_buffer.o: src/render/vulkan/vulkan_command_buffer.c
$(BUILDDIR)/vulkan_fence.o: src/render/vulkan/vulkan_fence.c
$(BUILDDIR)/vulkan_utils.o: src/render/vulkan/vulkan_utils.c
$(BUILDDIR)/vulkan_memory.o: src/render/vulkan/vulkan_memory.c
$(BUILDDIR)/vulkan_frontend.o: src/render/vulkan/vulkan_frontend.c
$(BUILDDIR)/vulkan_pipeline.o: src/render/vulkan/vulkan_pipeline.c
$(BUILDDIR)/render.o: src/render/render.c
$(BUILDDIR)/memcpy.o: src/asm/memcpy.asm
$(BUILDDIR)/memset.o: src/asm/memset.asm
$(BUILDDIR)/memeq.o: src/asm/memeq.asm

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
	-rm -rf $(BUILDDIR)
	-rm assets/shaders/*.spv

