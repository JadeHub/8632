CFLAGS=-m32 -std=gnu99 -I ../libc/include -Werror -ffreestanding -fno-exceptions
CC = i686-elf-gcc

C_SOURCES = $(wildcard *.c)
OBJS = ${C_SOURCES:.c=.o}

all: user_space

user_space: $(OBJS) 
	$(CC) -T link.ld -o $@  $< ../libc/libc.a -ffreestanding -O2 -nostdlib -lgcc

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

clean:
	rm user_space
	rm *.o
