CC = gcc
LD = ld

ROOTFS_BOOT := ../rootFileSystem/boot
ISO_BOOT := ../iso/boot

CFLAGS = -m64 -ffreestanding -nostdlib -fno-pie -fno-builtin
LDFLAGS = -n -T ../linker.ld

KERNEL_SRC := kernel.c idt.c isr.S
KERNEL_OBJ := $(KERNEL_SRC:.c=.o)
KERNEL_OBJ := $(KERNEL_OBJ:.S=.o)

BOOTLOADER_OBJ := ../boot/bootloader.o
KERNEL_ELF := $(ISO_BOOT)/kernel.elf

quiet_cmd_cc = CC      $<
      cmd_cc = $(CC) $(CFLAGS) -c $< -o $@

quiet_cmd_asm = ASM     $<
      cmd_asm = $(CC) $(CFLAGS) -c $< -o $@

quiet_cmd_ld = LD      $(KERNEL_ELF)
      cmd_ld = $(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(BOOTLOADER_OBJ) $(KERNEL_OBJ)

quiet_cmd_copy = COPY    kernel.elf
      cmd_copy = cp $(KERNEL_ELF) $(ROOTFS_BOOT)/kernel.elf

all: $(KERNEL_ELF)

%.o: %.c
	@echo "$(quiet_cmd_cc)"
	@$(cmd_cc)

%.o: %.S
	@echo "$(quiet_cmd_asm)"
	@$(cmd_asm)

$(KERNEL_ELF): $(KERNEL_OBJ) $(BOOTLOADER_OBJ)
	@mkdir -p $(ISO_BOOT) $(ROOTFS_BOOT)
	@echo "$(quiet_cmd_ld)"
	@$(cmd_ld)
	@echo "$(quiet_cmd_copy)"
	@$(cmd_copy)

clean:
	rm -f *.o
	rm -f $(ROOTFS_BOOT)/kernel.elf
