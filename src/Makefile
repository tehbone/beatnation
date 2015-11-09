OBJS :=\
	camera.o\
	collision.o\
	controller.o\
	gameapp.o\
	gfx.o\
	level.o\
	light.o\
	main_linux.o\
	model.o\
	objects.o\
	physics.o\
	targa.o\
	utils.o

BINARY = beatnation

CXXFLAGS += -g $(shell sdl2-config --cflags)
LIBS := -lGL -lGLU $(shell sdl2-config --libs)
.PHONY: all clean

all: $(BINARY)

$(BINARY): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(BINARY) $^ $(LIBS)

clean:
	-rm -f $(OBJS) $(BINARY)

