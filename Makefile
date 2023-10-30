CFLAGS=-O2 -march=native -pipe -Isrc

SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

main: $(OBJECTS)

clean:
	rm -f ${OBJECTS}
