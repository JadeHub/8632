CFLAGS=-m32 -std=gnu99 -I . -I ../libc/include -Werror -ffreestanding -fno-exceptions
CC = i686-elf-gcc

C_SOURCES = $(wildcard \
*.c \
commands/*.c)

OBJS = ${C_SOURCES:.c=.o}
LIBS = ../libc/libc.a

all: shell

shell: $(OBJS) $(LIBS)
	$(CC) -T link.ld -o $@  ${OBJS} $(LIBS) -ffreestanding -O2 -nostdlib -lgcc

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@

clean:
	rm -f shell
	rm -f *.o
	rm -f commands/*.o
