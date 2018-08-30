CFLAGS =  -std=gnu99 -Wall -Wextra -pedantic -O3 -MMD -MP
CFLAGS += $(shell pkg-config --cflags sdl2)
CFLAGS += $(shell pkg-config --cflags SDL2_image)
CFLAGS += $(shell pkg-config --cflags libxml-2.0)

CPPFLAGS = -DNDEBUG

LDLIBS =   $(shell pkg-config --libs sdl2)
LDLIBS +=  $(shell pkg-config --libs SDL2_image)
LDLIBS +=  $(shell pkg-config --libs libxml-2.0)
LDLIBS += -lm # for math

sources =  $(wildcard *.c)
sources += $(wildcard ./tmx/*.c)

objects = $(patsubst %.c,%.o,$(sources))

all: main
main: $(objects)

debug: all
debug: CPPFLAGS = -UNDEBUG
debug: CFLAGS += -ggdb -Og
debug: LDFLAGS += -ggdb -Og

clean:
	$(RM) $(objects) $(objects:.o=.d) main

-include $(objects:.o=.d)

.PHONY: all clean
