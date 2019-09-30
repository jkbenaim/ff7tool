target=ff7tool
sources=$(wildcard *.c)
objects=$(sources:.c=.o)


UNAME_S := $(shell uname -s)
CFLAGS=-Wall -ggdb -std=gnu99 -Os -march=native
LDFLAGS=-lglut -lGLU -lGL -lm

ifeq ($(UNAME_S),IRIX64)
CFLAGS+=-I/usr/nekoware/include
LDFLAGS+=-L/usr/nekoware/lib -Wl,-rpath,/usr/nekoware/lib
endif

.PHONY: all
all: $(target)

.PHONY: clean
clean:
	rm -f $(target) $(objects)

.PHONY: run
run:	$(target)
	./$(target) world wm0.map

$(target):	$(objects)
	$(CC) ${CFLAGS} $(objects) $(LDFLAGS) -o $(target)
