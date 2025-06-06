#!/bin/sh
cd ./shaders
test -d ../assets/shaders || mkdir -p ../assets/shaders
for f in *.vert ; do glslc -fshader-stage=vert --target-env=vulkan1.4 -o ../assets/shaders/"$f".spv "$f" || exit 1 ; echo "$f" ; done
for f in *.frag ; do glslc -fshader-stage=frag --target-env=vulkan1.4 -o ../assets/shaders/"$f".spv "$f" || exit 1 ; echo "$f" ; done
for f in *.comp ; do glslc -fshader-stage=comp --target-env=vulkan1.4 -o ../assets/shaders/"$f".spv "$f" || exit 1 ; echo "$f" ; done
