AS := nasm
CC := gcc
LD := ld

ISO := myos.iso
KERNEL_ELF := iso/boot/kernel.elf
ROOTFS_KERNEL := ../rootFileSystem/boot/kernel.elf

CFLAGS := -m64 -ffreestanding -nostdlib -fno-pie -fno-builtin
LDFLAGS := -n -T linker.ld

.PHONY: all clean install-kernel run iso

all: $(ISO) install-kernel

# ---------------------------------------------------------
#  BUILD KERNEL ELF
# ---------------------------------------------------------

$(KERNEL_ELF): kernel_entry.S kernel.c linker.ld grub.cfg
	@mkdir -p iso/boot/grub

	@printf "  AS      %s\n" "kernel_entry.S"
	@$(AS) -f elf64 kernel_entry.S -o kernel_entry.o

	@printf "  CC      %s\n" "kernel.c"
	@$(CC) $(CFLAGS) -c kernel.c -o kernel.o

	@printf "  LD      %s\n" "$(KERNEL_ELF)"
	@$(LD) $(LDFLAGS) -o $(KERNEL_ELF) kernel_entry.o kernel.o

	@printf "  CP      %s → iso/boot/grub/grub.cfg\n" "grub.cfg"
	@cp grub.cfg iso/boot/grub/grub.cfg

# ---------------------------------------------------------
#  GENERATE ISO
# ---------------------------------------------------------

$(ISO): $(KERNEL_ELF)
	@printf "  MKISO   %s\n" "$(ISO)"
	@grub-mkrescue -o $(ISO) iso

# ---------------------------------------------------------
#  INSTALL KERNEL TO rootFileSystem
# ---------------------------------------------------------

install-kernel: $(KERNEL_ELF)
	@mkdir -p ../rootFileSystem/boot
	@printf "  INSTALL %s → %s\n" "$(KERNEL_ELF)" "$(ROOTFS_KERNEL)"
	@cp $(KERNEL_ELF) $(ROOTFS_KERNEL)

# ---------------------------------------------------------
#  RUN VIA QEMU
# ---------------------------------------------------------

run: $(ISO)
	qemu-system-x86_64 \
		-cdrom $(ISO) \
		-m 1024 \
		-no-reboot \
		-nographic \
		-serial mon:stdio \
		-accel tcg,thread=single

# ---------------------------------------------------------
#  CLEAN BUILD OUTPUT
# ---------------------------------------------------------

clean:
	@printf "  CLEAN\n"
	@rm -rf iso *.o $(ISO)