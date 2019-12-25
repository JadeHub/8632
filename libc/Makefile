CFLAGS=-m32 -std=gnu99 -I ./include -Werror
LIBK_CFLAGS:=$(CFLAGS)  -ffreestanding
AR = i386-elf-ar

C_SOURCES = $(wildcard \
string/*.c \
ctype/*.c \
stdlib/*.c)

HEADERS = $(wildcard include/*.h)

LIBK_OBJS = ${C_SOURCES:.c=.o}

#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES=libk.a

all: $(BINARIES)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

%.o: %.c
	$(CC) ${LIBK_CFLAGS} -c $< -o $@ 

clean:
	rm -f -r *.bin *.o *.a
	find . -name "*.o" | xargs rm -f
	find . -name "*.d" | xargs rm -f