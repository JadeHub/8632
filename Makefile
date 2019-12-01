CFLAGS=-m32 -std=gnu99 -fno-exceptions -ffreestanding -I ./

C_SOURCES = $(wildcard \
kernel/*.c \
kernel/memory/*.c \
kernel/x86/*.c \
drivers/*.c \
drivers/keyboard/*.c \
drivers/timer/*.c)

HEADERS = $(wildcard \
kernel/*.h \
kernel/memory/*.h \
kernel/x86/*.h \
drivers/*.h \
drivers/keyboard/*.h \
drivers/timer/*.h)

ASM_SOURCES = $(wildcard kernel/x86/*.s \
kernel/memory/*.s \
drivers/*.s)

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
		~/gcc_i386/bin/i386-elf-gcc ${CFLAGS} -c $< -o $@

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
		rm -f -r *.bin *.dis *.o os-image
		find . -name "*.o" | xargs rm -f
		
