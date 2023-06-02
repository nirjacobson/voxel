MODULES=global       \
		      application  \
								linked_list		\
		      world		      \
		      ground		     \
		      camera       \
		      mesh         \
		      renderer     \
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

build/%.o : build/ src/%.c
	gcc -c $(word 2, $^) -o $@ ${CFLAGS}

build/:
	mkdir build

format:
	astyle -rnNCS *.{c,h}

clean:
	rm -rf build
	rm ${EXEC}