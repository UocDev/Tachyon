AS := nasm
CC := gcc
LD := ld

ISO := myos.iso
KERNEL_ELF := iso/boot/kernel.elf
ROOTFS_KERNEL := ../rootFileSystem/boot/kernel.elf

CFLAGS := -m64 -ffreestanding -nostdlib -fno-pie -fno-builtin
LDFLAGS := -n -T linker.ld

.PHONY: all clean install-kernel run iso

# =======================================
# Build total
# =======================================
all: $(ISO) install-kernel

# =======================================
# Build kernel ELF
# =======================================
$(KERNEL_ELF): kernel_entry.S kernel.c linker.ld grub.cfg
	@mkdir -p iso/boot/grub
	$(AS) -f elf64 kernel_entry.S -o kernel_entry.o
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) kernel_entry.o kernel.o
	cp grub.cfg iso/boot/grub/grub.cfg

# =======================================
# Build ISO dengan GRUB
# =======================================
$(ISO): $(KERNEL_ELF)
	grub-mkrescue -o $(ISO) iso

# =======================================
# Install kernel ke root filesystem
# =======================================
install-kernel: $(KERNEL_ELF)
	@mkdir -p ../rootFileSystem/boot
	cp $(KERNEL_ELF) $(ROOTFS_KERNEL)
	@echo "[INSTALL] Kernel installed → rootFileSystem/boot/kernel.elf"

# =======================================
# Run OS di QEMU
# =======================================
run: $(ISO)
	qemu-system-x86_64 \
		-cdrom $(ISO) \
		-m 1024 \
		-no-reboot \
		-nographic \
		-serial mon:stdio \
		-accel tcg,thread=single

# =======================================
# Clean build hasil
# =======================================
clean:
	rm -rf iso *.o $(ISO)
