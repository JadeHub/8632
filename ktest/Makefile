CFLAGS=-m32 -std=gnu99 -I ../libc/include -Werror -ffreestanding -fno-exceptions
LDDFLAGS=-ffreestanding -O2 -nostdlib -lgcc -Tlink.ld 
CC = i686-elf-gcc

C_SOURCES = $(wildcard src/*.c)

LIBS = ../libc/libc.a

OBJS = ${C_SOURCES:.c=.o}

all: bin/ktest

bin/ktest: ${OBJS} $(LIBS)
	$(CC) -o $@ ${OBJS} $(LIBS) ${LDDFLAGS}

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

clean:
	rm -f bin/ktest
	rm -f src/*.o
