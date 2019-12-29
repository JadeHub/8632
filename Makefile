CFLAGS=-m32 -std=gnu99 -I ./ -Werror

C_SOURCES = $(wildcard \
kernel/*.c \
kernel/memory/*.c \
kernel/x86/*.c \
kernel/tasks/*.c \
kernel/dbg_monitor/*.c \
kernel/sync/*.c \
kernel/elf32/*.c \
kernel/fs/*.c \
kernel/ramfs/*.c \
drivers/*.c \
drivers/keyboard/*.c \
drivers/ata/*.c \
drivers/timer/*.c \
drivers/serial/*.c)

HEADERS = $(wildcard \
kernel/*.h \
kernel/memory/*.h \
kernel/x86/*.h \
kernel/tasks/*.h \
kernel/dbg_monitor/*.h \
kernel/sync/*.h \
kernel/elf32/*.h \
kernel/fs/*.h \
kernel/ramfs/*.h \
drivers/*.h \
drivers/keyboard/*.h \
drivers/ata/*.h \
drivers/timer/*.h \
drivers/serial/*.h)

ASM_SOURCES = $(wildcard kernel/x86/*.s \
kernel/memory/*.s \
kernel/tasks/*.s \
kernel/sync/*.s \
drivers/*.s)

KERNEL_OBJ = ${C_SOURCES:.c=.o}
ASM_OBJ = ${ASM_SOURCES:.s=.o}

$(KERNEL_OBJ): K_CFLAGS := -fno-exceptions -ffreestanding 

all: kernel.iso

bochs: all
		bochs

kernel.iso: kernel.bin
		./make_iso.sh

kernel.bin: multiboot/multiboot.o ${KERNEL_OBJ} ${ASM_OBJ} libc/libk.a
		i686-elf-gcc -T link.ld -o kernel.bin $^ -ffreestanding -O2 -nostdlib -lgcc 

%.o: %.c ${HEADERS}
		i686-elf-gcc ${CFLAGS} ${K_CFLAGS} -c $< -o $@ -I libc/include

%.o: %.s
		nasm $< -f elf32 -o $@

boot/boot_sect.bin: boot/boot_sect.asm
		nasm $< -f bin -o $@

multiboot/multiboot.o: multiboot/multiboot.asm
		nasm $< -felf -o $@

%.bin: %.o
		nasm $< -f bin -o $@

clean:
		rm -f -r *.bin *.dis *.o os-image
		find . -name "*.o" | xargs rm -f
		
