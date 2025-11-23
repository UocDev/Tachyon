# ============ DIRECTORIES ============
BOOT_DIR := boot
KERNEL_DIR := kernel
ISO := release/myos.iso

.PHONY: all boot kernel iso run clean

all: boot kernel iso

boot:
	@$(MAKE) -C $(BOOT_DIR)

kernel:
	@$(MAKE) -C $(KERNEL_DIR)

iso:
	@echo "ISO     $(ISO)"
	@mkdir -p release
	grub-mkrescue -o $(ISO) iso >/dev/null 2>&1

run:
	qemu-system-x86_64 \
		-cdrom $(ISO) \
		-m 1720 \
		-no-reboot \
		-nographic

clean:
	@echo "CLEAN   boot/"
	@$(MAKE) -C $(BOOT_DIR) clean
	@echo "CLEAN   kernel/"
	@$(MAKE) -C $(KERNEL_DIR) clean
	@echo "CLEAN   release/"
	rm -rf release iso
