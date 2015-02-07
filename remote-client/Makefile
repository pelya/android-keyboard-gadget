SRCS=$(wildcard *.cpp *.c)
HDRS=$(wildcard *.h)

usb-keyboard-client: $(SRCS)
	g++ -g -o $@ $+ -lSDL_ttf -lSDL_image `sdl-config --cflags` `sdl-config --libs`
	cp AndroidData/*.ttf .
	unzip -n AndroidData/images*.zip

libapplication-$(ARCH).so: $(OBJS)
	$(CXX) -o $@ $+ -lsdl_ttf -lsdl_image -lsdl-1.2 -ljpeg -lpng -lz -llog

OBJS=$(patsubst %.cpp, obj/%.o, $(patsubst %.c, obj/%.o, $(SRCS)))
$(OBJS): obj

define SRC2OBJ
$$(patsubst %.cpp, obj/%.o, $$(patsubst %.c, obj/%.o, $$(SRC))): obj $$(SRC) $(HDRS)
	$$(CXX) -o $@ $$(SRC)
endef

$(foreach SRC, $(SRCS), $(eval $(SRC2OBJ)))

obj:
	mkdir -p $@
