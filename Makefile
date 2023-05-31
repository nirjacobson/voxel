MODULES = global       \
          application  \
          window       \
          voxel        \
          main
OBJECTS = $(foreach MODULE, ${MODULES}, build/${MODULE}.o)
LIBS    = gl glfw3
CFLAGS  = -O2 -Wall -Wno-unused-result `pkg-config --cflags ${LIBS}` -g
LDFLAGS = `pkg-config --libs ${LIBS}` -lm
EXEC    = voxel

${EXEC}: ${OBJECTS}
	gcc $^ -o $@ ${LDFLAGS}

format:
	astyle -rnNCS *.{c,h}

build/%.o : build/ src/%.c
	gcc -c $(word 2, $^) -o $@ ${CFLAGS}

build/:
	mkdir -p build

clean:
	rm -rf build
	rm ${EXEC}