CC = gcc
LD = ld

ROOTFS_BOOT := ../rootFileSystem/boot
ISO_BOOT := ../iso/boot

CFLAGS = -m64 -ffreestanding -nostdlib -fno-pie -fno-builtin
LDFLAGS = -n -T ../linker.ld

KERNEL_SRC := kernel.c
KERNEL_OBJ := kernel.o
BOOTLOADER_OBJ := ../boot/bootloader.o
KERNEL_ELF := $(ISO_BOOT)/kernel.elf

# =======================
# Pretty build (Kbuild)
# =======================
quiet_cmd_cc = CC      $<
      cmd_cc = $(CC) $(CFLAGS) -c $< -o $@

quiet_cmd_ld = LD      $(KERNEL_ELF)
      cmd_ld = $(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(BOOTLOADER_OBJ) $(KERNEL_OBJ)

quiet_cmd_copy = COPY    rootFileSystem/boot/kernel.elf
      cmd_copy = cp $(KERNEL_ELF) $(ROOTFS_BOOT)/kernel.elf

.PHONY: all clean

all: $(KERNEL_ELF)

$(KERNEL_OBJ): $(KERNEL_SRC)
	@echo "$(quiet_cmd_cc)"
	@$(cmd_cc)

$(KERNEL_ELF): $(KERNEL_OBJ) $(BOOTLOADER_OBJ)
	@mkdir -p $(ISO_BOOT) $(ROOTFS_BOOT)
	@echo "$(quiet_cmd_ld)"
	@$(cmd_ld)
	@echo "$(quiet_cmd_copy)"
	@$(cmd_copy)

clean:
	rm -f $(KERNEL_OBJ)
	rm -f $(ROOTFS_BOOT)/kernel.elf
