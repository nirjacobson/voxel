MODULES = global       \
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

format:
	astyle -rnNCS *.{c,h}

build/:
	mkdir -p build

build/%.o : src/%.c | build/
	gcc -c $< -o $@ ${CFLAGS}

clean:
	rm -rf build
	rm ${EXEC}