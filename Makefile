MODULES = global       \
          commands/world_clear_region_command   \
          commands/world_copy_chunk_command     \
          commands/world_cut_chunk_command      \
          commands/world_set_region_command     \
          shaders/3D.vert \
          shaders/3D.frag \
          shaders/2D.vert \
          shaders/2D.frag \
          undo_stack   \
          vulkan_util  \
          panel        \
          fps_panel    \
          picker       \
          picker_panel \
          box          \
          application  \
          linked_list  \
          bp_tree      \
          heap         \
          chunk_dao    \
          world        \
          ground       \
          camera       \
          block        \
          chunk        \
          mesh         \
          renderer     \
          matrix       \
          window       \
          voxel        \
          main
OBJECTS = $(foreach MODULE, ${MODULES}, build/${MODULE}.o)
LIBS    = vulkan glfw3 cairo
CFLAGS  = -O2 -Wall -Wno-unused-result `pkg-config --cflags ${LIBS}` -g
LDFLAGS = `pkg-config --libs ${LIBS}` -lm
EXEC    = voxel

${EXEC}: ${OBJECTS}
	gcc $^ -o $@ ${LDFLAGS}

%.vert.spv : %.vert.glsl
	glslc -fshader-stage=vert -c $< -o $@

%.frag.spv : %.frag.glsl
	glslc -fshader-stage=frag -c $< -o $@

src/shaders/%.c: src/shaders/%.spv
	xxd -i -n $(notdir $<) $< $@
	sed -i 's/unsigned/const unsigned/g' $@

format:
	astyle -rnNCS *.{c,h}

build/:
	mkdir -p build/commands
	mkdir -p build/shaders

build/%.o : src/%.c | build/
	gcc -c $< -o $@ ${CFLAGS}

clean:
	rm -rf build
	rm -rf src/shaders/*.{spv,c}
	rm ${EXEC}