MODULES=global       \
		      panel		      \
		      picker_panel \
		      picker							\
		      application  \
		      bp_tree	     \
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
		      window       \
		      voxel        \
		      fps_panel    \
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

clean:
	rm -rf build
	rm ${EXEC}