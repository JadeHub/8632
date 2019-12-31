CFLAGS=-m32 -std=gnu99 -I ./include -Werror -ffreestanding
AR = i686-elf-ar
CC = i686-elf-gcc

C_SOURCES = $(wildcard \
string/*.c \
ctype/*.c \
stdlib/*.c)

HEADERS = $(wildcard include/*.h)

OBJS = ${C_SOURCES:.c=.o}
LIBC_OBJS=${OBJS}
LIBK_OBJS=$(OBJS:.o=.libk.o)

BINARIES=libk.a libc.a

all: $(BINARIES)

libc: libc.a

libk: libk.a

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

libc.a: $(LIBC_OBJS)
	$(AR) rcs $@ $(LIBC_OBJS)

%.o: %.c
	$(CC) ${CFLAGS} -c $< -o $@ 

%.libk.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -f -r *.bin *.o *.a
	find . -name "*.o" | xargs rm -f
	find . -name "*.d" | xargs rm -f