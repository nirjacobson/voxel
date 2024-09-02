MODULES = global           \
		  resources        \
		  commands/world_set_region_command \
		  commands/world_clear_region_command \
		  commands/world_copy_chunk_command \
		  commands/world_cut_chunk_command \
		  undo_stack       \
		  panel            \
		  fps_panel        \
		  picker           \
		  picker_panel     \
		  box              \
		  application      \
		  linked_list      \
		  bp_tree          \
		  heap             \
		  chunk_dao        \
		  world            \
		  ground           \
		  camera           \
		  block            \
		  chunk            \
		  mesh             \
		  renderer         \
		  shader           \
		  matrix           \
		  window           \
		  voxel            \
		  vulkan_util      \
		  main
SHADERS = 3D.vert 3D.frag 2D.vert 2D.frag          
OBJECTS = $(foreach MODULE, ${MODULES}, build/${MODULE}.o)
LIBS    = vulkan gl glfw3 cairo gio-2.0
CFLAGS  = -O2 -Wall -Wno-unused-result `pkg-config --cflags ${LIBS}` -g
LDFLAGS = `pkg-config --libs ${LIBS}` -lm
EXEC    = voxel

.SECONDARY:

${EXEC}: ${OBJECTS}
	gcc $^ -o $@ ${LDFLAGS}

build/shaders/%.vert.spv : src/shaders/glsl/450/%.vert.glsl
	glslc -fshader-stage=vert -c $< -o $@

build/shaders/%.frag.spv : src/shaders/glsl/450/%.frag.glsl
	glslc -fshader-stage=frag -c $< -o $@

src/resources.c: resources.xml $(foreach SHADER, ${SHADERS}, src/shaders/glsl/300/${SHADER}.glsl) $(foreach SHADER, ${SHADERS}, build/shaders/${SHADER}.spv)
	glib-compile-resources --target=$@ --generate-source $<

format:
	astyle -rnNCS *.{c,h}

build/:
	mkdir -p build/commands
	mkdir -p build/shaders

build/%.o : src/%.c | build/
	gcc -c $< -o $@ ${CFLAGS}

clean:
	rm -f src/resources.c
	rm -rf build
	rm ${EXEC}