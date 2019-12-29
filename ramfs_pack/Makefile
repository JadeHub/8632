CFLAGS=-std=gnu99 -I ../ 

PACK_SOURCES = ramfs_pack.c

PACK_OBJS = ${PACK_SOURCES:.c=.o}

all: ramfs_pack

ramfs_pack: ${PACK_OBJS}
		gcc -o ramfs_pack $^ -O2 -lgcc 

%.o: %.c
		gcc ${CFLAGS} -c $< -o $@

clean:
		rm -f -r ramfs_pack *.o 
	