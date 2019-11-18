MODULES=global        \
		panel		  \
		picker_panel  \
		picker		  \
		cursor		  \
		b_tree		  \
		heap		  \
		chunk_dao	  \
		world		  \
		ground		  \
		box           \
		camera        \
		block         \
		chunk         \
		mesh          \
		renderer      \
		linked_list   \
		shader matrix \
		keyboard      \
		window        \
		mouse         \
		voxel         \
		main
OBJECTS=$(foreach MODULE, ${MODULES}, build/${MODULE}.o)
CFLAGS=-Wall -I/opt/vc/include `pkg-config --cflags cairo`
LDFLAGS+=-L/opt/vc/lib/ -lbrcmGLESv2 -lbrcmEGL -lbcm_host -lm  `pkg-config --libs cairo`
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