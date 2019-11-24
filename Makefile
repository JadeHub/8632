CFLAGS=-m32

C_SOURCES = $(wildcard kernel/*.c drivers/*.c)
HEADERS = $(wildcard kernel/*.h drivers/*.h)
ASM_SOURCES = $(wildcard kernel/*.s drivers/*.s)

OBJ = ${C_SOURCES:.c=.o}
ASM_OBJ = ${ASM_SOURCES:.s=.o}

all: os-image

run: all
		qemu-system-i386 -fda os-image

bochs: all
		bochs

os-image: boot/boot_sect.bin boot/stage2.bin kernel.bin
		cat $^ > os-image

kernel.bin: kernel/kernel_entry.o ${OBJ} ${ASM_OBJ}
		 ~/gcc_i386/i386-elf/bin/ld -o $@ -Tlink.ld  $^ --oformat binary

%.o: %.c ${HEADERS}
		~/gcc_i386/bin/i386-elf-gcc -std=gnu99 -fno-exceptions ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
		nasm $< -f elf32 -o $@

%.o: %.s
		nasm $< -f elf32 -o $@

boot/boot_sect.bin: boot/boot_sect.asm
		nasm $< -f bin -o $@

boot/stage2.bin: boot/stage2.asm
		nasm $< -f bin -o $@	

%.bin: %.o
		nasm $< -f bin -o $@

clean:
		rm -fr *.bin *.dis *.o os-image
		rm -fr kernel/*.o boot/*.bin drivers/*.o
