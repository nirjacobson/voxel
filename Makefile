MODULES = global           \
          commands/world_set_region_command \
          commands/world_clear_region_command \
          commands/world_copy_chunk_command \
          commands/world_cut_chunk_command \
          shaders/3D.vert  \
          shaders/3D.frag  \
          shaders/2D.vert  \
          shaders/2D.frag  \
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
          main
OBJECTS = $(foreach MODULE, ${MODULES}, build/${MODULE}.o)
LIBS    = gl glfw3 cairo
CFLAGS  = -O2 -Wall -Wno-unused-result `pkg-config --cflags ${LIBS}` -g
LDFLAGS = `pkg-config --libs ${LIBS}` -lm
EXEC    = voxel

${EXEC}: ${OBJECTS}
	gcc $^ -o $@ ${LDFLAGS}

src/shaders/%.c: src/shaders/%.glsl
	xxd -i -n $(notdir $<) $< $@
	sed -i 's/unsigned/const/g' $@

format:
	astyle -rnNCS *.{c,h}

build/:
	mkdir -p build/commands
	mkdir -p build/shaders

build/%.o : src/%.c | build/
	gcc -c $< -o $@ ${CFLAGS}

clean:
	rm -rf src/shaders/*.c
	rm -rf build
	rm ${EXEC}