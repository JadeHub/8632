CFLAGS=-m32 -std=gnu99 -I . -I ../libc/include -Werror -ffreestanding -fno-exceptions
CC = i686-elf-gcc

C_SOURCES = $(wildcard \
*.c \
commands/*.c)

OBJS = ${C_SOURCES:.c=.o}

all: shell

shell: $(OBJS) 
	$(CC) -T link.ld -o $@  ${OBJS} ../libc/libc.a -ffreestanding -O2 -nostdlib -lgcc

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

clean:
	rm shell
	rm *.o
