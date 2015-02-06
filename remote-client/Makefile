SRCS=$(wildcard *.cpp *.c)

usb-keyboard-client: $(SRCS)
	g++ -g -o $@ $+ -lSDL_ttf -lSDL_image `sdl-config --cflags` `sdl-config --libs`
