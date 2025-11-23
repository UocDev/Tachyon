AS = nasm

ISO_BOOT := ../iso/boot
ISO_GRUB := ../iso/boot/grub

BOOTLOADER_SRC := bootloader.S
BOOTLOADER_OBJ := bootloader.o

quiet_cmd_as = ASM     $<
      cmd_as = $(AS) -f elf64 $< -o $@

quiet_cmd_copycfg = COPY    grub.cfg
      cmd_copycfg = cp grub.cfg $(ISO_GRUB)/grub.cfg

.PHONY: all clean

all: $(ISO_GRUB)/grub.cfg $(BOOTLOADER_OBJ)

$(BOOTLOADER_OBJ): $(BOOTLOADER_SRC)
	@mkdir -p $(ISO_BOOT) $(ISO_GRUB)
	@echo "$(quiet_cmd_as)"
	@$(cmd_as)

$(ISO_GRUB)/grub.cfg: grub.cfg
	@mkdir -p $(ISO_GRUB)
	@echo "$(quiet_cmd_copycfg)"
	@$(cmd_copycfg)

clean:
	@rm -f $(BOOTLOADER_OBJ)
