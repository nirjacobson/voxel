MODULES=global       \
		      picker							\
		      application  \
		      world		      \
		      ground		     \
		      camera       \
		      block        \
		      chunk        \
		      mesh         \
		      renderer     \
		      linked_list  \
		      shader       \
		      matrix       \
		      window       \
		      voxel        \
		      main
OBJECTS=$(foreach MODULE, ${MODULES}, build/${MODULE}.o)
LIBS    = gl glfw3 cairo
CFLAGS  = -O2 -Wall -Wno-unused-result `pkg-config --cflags ${LIBS}` -g
LDFLAGS = `pkg-config --libs ${LIBS}` -lm
EXEC=voxel

${EXEC}: ${OBJECTS}
	gcc $^ -o $@ ${LDFLAGS}

build/:
	mkdir -p build

build/%.o : src/%.c | build/
	gcc -c $< -o $@ ${CFLAGS}

format:
	astyle -rnNCS *.{c,h}

clean:
	rm -rf build
	rm ${EXEC}