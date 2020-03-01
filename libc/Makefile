CFLAGS=-m32 -std=gnu99 -I ./include -Werror -ffreestanding
AR = i686-elf-ar
CC = i686-elf-gcc

C_SOURCES = $(wildcard \
*.c \
string/*.c \
ctype/*.c \
stdlib/*.c \
stdio/*.c)

ASM_SOURCES = $(wildcard *.s)

HEADERS = $(wildcard include/*.h)

OBJS = ${C_SOURCES:.c=.o}
ASM_OBJ = ${ASM_SOURCES:.s=.o}
LIBC_OBJS=${OBJS}
LIBK_OBJS=$(OBJS:.o=.libk.o)

BINARIES=libk.a libc.a

all: $(BINARIES)

libc: libc.a

libk: libk.a

libk.a: $(LIBK_OBJS) ${ASM_OBJ}
	$(AR) rcs $@ $(LIBK_OBJS) ${ASM_OBJ}

libc.a: $(LIBC_OBJS) ${ASM_OBJ}
	$(AR) rcs $@ $(LIBC_OBJS) ${ASM_OBJ}

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@ 

%.libk.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ -D _LIBK

%.o: %.s
	nasm $< -f elf32 -o $@

clean:
	rm -f -r *.bin *.o *.a
	find . -name "*.o" | xargs rm -f
	find . -name "*.d" | xargs rm -f