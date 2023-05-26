MODULES=global       \
		      panel		      \
		      picker_panel \
		      picker							\
		      application  \
		      b_tree		     \
		      heap		       \
		      chunk_dao	   \
		      world		      \
		      ground		     \
		      box          \
		      camera       \
		      block        \
		      chunk        \
		      mesh         \
		      renderer     \
		      linked_list  \
		      shader       \
		      matrix       \
		      keyboard     \
		      window       \
		      mouse        \
		      voxel        \
		      fps_panel    \
		      main
OBJECTS=$(foreach MODULE, ${MODULES}, build/${MODULE}.o)
CFLAGS  = -O2 -Wall `pkg-config --cflags gl glfw3 cairo` -g
LDFLAGS = `pkg-config --libs gl glfw3 cairo` -lm
EXEC=voxel

all: build ${EXEC}

${EXEC}: ${OBJECTS}
	gcc $^ -o $@ ${LDFLAGS}

build/%.o : src/%.c
	gcc -c $< -o $@ ${CFLAGS}

build:
	mkdir build

clean:
	rm -rf build
	rm ${EXEC}