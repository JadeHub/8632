CFLAGS=-m32 -std=gnu99 -I ../libc/include -Werror -ffreestanding -fno-exceptions
LDDFLAGS=-ffreestanding -O2 -nostdlib -lgcc -Tlink.ld 
CC = i686-elf-gcc

LIBS = ../libc/libc.a

APPS = user_space cat

all: bin/user_space \
	bin/cat

bin/user_space: src/user_space.o $(LIBS)
	$(CC) -o $@ src/user_space.o $(LIBS) ${LDDFLAGS}

bin/cat: src/cat.o $(LIBS) 
	$(CC) -o $@ src/cat.o $(LIBS) ${LDDFLAGS}

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

clean:
	rm -f bin/user_space
	rm -f bin/cat
	rm -f src/*.o
