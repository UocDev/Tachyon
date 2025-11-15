AS := nasm
CC := gcc
LD := ld
OBJCOPY := objcopy

ISO := myos.iso
KERNEL_ELF := iso/boot/kernel.elf

.PHONY: all clean run iso install-deps

all: $(ISO)

install-deps:
	@echo "sudo apt update && sudo apt install -y build-essential nasm grub-pc-bin xorriso qemu-system-x86"

# ================================
# Build kernel ELF
# ================================
$(KERNEL_ELF): kernel_entry.S kernel.c linker.ld grub.cfg
	@mkdir -p iso/boot/grub
	$(AS) -f elf64 kernel_entry.S -o kernel_entry.o
	$(CC) -m64 -ffreestanding -nostdlib -fno-pie -fno-builtin -c kernel.c -o kernel.o
	$(LD) -n -T linker.ld -o $(KERNEL_ELF) kernel_entry.o kernel.o
	cp grub.cfg iso/boot/grub/grub.cfg

# ================================
# Build ISO
# ================================
$(ISO): $(KERNEL_ELF)
	grub-mkrescue -o $(ISO) iso

# ================================
# Run with QEMU headless
# ================================
run: $(ISO)
	qemu-system-x86_64 \
		-cdrom $(ISO) \
		-m 1720 \
		-no-reboot \
		-nographic \
		-accel tcg,thread=single

# ================================
# Clean
# ================================
clean:
	rm -rf iso *.o $(ISO)
