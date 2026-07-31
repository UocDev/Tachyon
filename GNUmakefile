# ============================================================================
# Konfigurasi dasar
# ============================================================================
SHELL := /bin/bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

# Direktori output
BUILD_DIR ?= build
OBJ_DIR   := $(BUILD_DIR)/obj

# ============================================================================
# Deteksi arsitektur
# ============================================================================
ARCH ?= $(shell uname -m)
ARCH := $(ARCH)

# Pilih toolchain berdasarkan arsitektur
ifeq ($(ARCH), x86_64)
    CROSS_COMPILE ?= x86_64-linux-gnu-
else ifeq ($(ARCH), i386)
    CROSS_COMPILE ?= i686-linux-gnu
else ifeq ($(ARCH), i686)
    CROSS_COMPILE ?= i686-linux-gnu
else ifeq ($(ARCH), arm)
    CROSS_COMPILE ?= arm-linux-gnu-
else ifeq ($(ARCH), aarch64)
    CROSS_COMPILE ?= aarch64-linux-gnu-
else
    $(error Unsupported architecture: $(ARCH))
endif

# Tools
CC  := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AS  := $(CROSS_COMPILE)as
LD  := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

# ============================================================================
# Bootloader dan Linker Script
# ============================================================================
BOOTLOADER_S := boot/bootloader_$(ARCH).S
LINKER_SCRIPT := linker/linker_$(ARCH).ld

ifeq ($(wildcard $(BOOTLOADER_S)),)
    $(error Bootloader file $(BOOTLOADER_S) not found)
endif
ifeq ($(wildcard $(LINKER_SCRIPT)),)
    $(error Linker script $(LINKER_SCRIPT) not found)
endif

# ============================================================================
# Flags
# ============================================================================
# C/C++ flags (sesuaikan dengan target OS)
CFLAGS   := -std=gnu11 -ffreestanding -nostdlib -Wall -Wextra -O2 -g
CXXFLAGS := -std=gnu++17 -ffreestanding -nostdlib -fno-rtti -fno-exceptions -Wall -Wextra -O2 -g
ASFLAGS  := --64   # untuk x86_64, sesuaikan untuk arch lain
LDFLAGS  := -nostdlib -static -z max-page-size=0x1000

# Tambahkan flag untuk arsitektur spesifik
ifeq ($(ARCH), x86_64)
    CFLAGS   += -m64 -mno-red-zone -fno-stack-protector -mcmodel=large
    CXXFLAGS += -m64 -mno-red-zone -fno-stack-protector -mcmodel=large
    ASFLAGS  := --64
    LDFLAGS  += -m elf_x86_64
else ifeq ($(ARCH), i386)
    CFLAGS   += -m32 -mno-red-zone -fno-stack-protector
    CXXFLAGS += -m32 -mno-red-zone -fno-stack-protector
    ASFLAGS  := --32
    LDFLAGS  += -m elf_i386
else ifeq ($(ARCH), arm)
    CFLAGS   += -marm -mcpu=cortex-a8 -mfloat-abi=soft
    CXXFLAGS += -marm -mcpu=cortex-a8 -mfloat-abi=soft
    ASFLAGS  := -march=armv7-a
    LDFLAGS  += -m armelf
endif

# Flags untuk generate dependensi
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

# ============================================================================
# Kumpulkan semua source (kecuali direktori build)
# ============================================================================
SRCS := $(shell find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.S" \) -not -path "*/$(BUILD_DIR)/*")

# Ubah menjadi objek dengan path yang sama di dalam $(OBJ_DIR)
OBJS := $(patsubst ./%,$(OBJ_DIR)/%,$(SRCS))
OBJS := $(patsubst %.c,%.o,$(OBJS))
OBJS := $(patsubst %.cpp,%.o,$(OBJS))
OBJS := $(patsubst %.S,%.o,$(OBJS))

# Daftar direktori sumber untuk VPATH
SRC_DIRS := $(sort $(dir $(SRCS)))
VPATH := $(SRC_DIRS)

# ============================================================================
# Target utama
# ============================================================================
.PHONY: all clean distclean iso run submodules

all: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin

# Build kernel ELF
$(BUILD_DIR)/kernel.elf: $(OBJS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $(OBJS)
	$(OBJDUMP) -d $@ > $(BUILD_DIR)/kernel.asm

# Build kernel binary
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) -O binary $< $@

# ============================================================================
# Aturan kompilasi per tipe file
# ============================================================================
# Compile C
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Compile C++
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Compile Assembly (dengan preprocessor)
$(OBJ_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# ============================================================================
# Dependensi otomatis
# ============================================================================
DEPS := $(OBJS:.o=.d)
-include $(DEPS)

# ============================================================================
# Submodule support
# ============================================================================
SUBMODULES := $(shell find . -mindepth 2 -maxdepth 2 -type f -name "Makefile" -printf "%h\n" | sort -u)

submodules:
	@for dir in $(SUBMODULES); do \
		echo "Making in $$dir"; \
		$(MAKE) -C $$dir; \
	done

# Jika ingin include sub-Makefile (misal untuk menambahkan aturan tambahan)
# include $(wildcard */Makefile)  # hati-hati dengan konflik

# ============================================================================
# ISO Image (untuk boot dengan GRUB)
# ============================================================================
ISO_DIR := $(BUILD_DIR)/iso
GRUB_CFG := $(ISO_DIR)/boot/grub/grub.cfg

$(GRUB_CFG):
	@mkdir -p $(dir $@)
	@echo "set timeout=0" > $@
	@echo "set default=0" >> $@
	@echo "menuentry \"MyOS\" {" >> $@
	@echo "  multiboot2 /boot/kernel.elf" >> $@
	@echo "  boot" >> $@
	@echo "}" >> $@

iso: $(BUILD_DIR)/kernel.elf $(GRUB_CFG)
	@mkdir -p $(ISO_DIR)/boot
	cp $(BUILD_DIR)/kernel.elf $(ISO_DIR)/boot/
	grub2-mkrescue -o $(BUILD_DIR)/os.iso $(ISO_DIR)

# ============================================================================
# Jalankan dengan QEMU
# ============================================================================
QEMU := qemu-system-$(ARCH)
ifeq ($(ARCH), x86_64)
    QEMU_OPTS := -cdrom $(BUILD_DIR)/os.iso -m 256M -serial mon:stdio
else ifeq ($(ARCH), i386)
    QEMU_OPTS := -cdrom $(BUILD_DIR)/os.iso -m 256M -serial mon:stdio
else ifeq ($(ARCH), arm)
    QEMU_OPTS := -kernel $(BUILD_DIR)/kernel.elf -M virt -m 256M -nographic
endif

run: iso
	$(QEMU) $(QEMU_OPTS)

# ============================================================================
# Pembersihan
# ============================================================================
clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(BUILD_DIR)/iso
	find . -name "*.d" -delete

# ============================================================================
# Informasi
# ============================================================================
info:
	@echo "ARCH = $(ARCH)"
	@echo "CROSS_COMPILE = $(CROSS_COMPILE)"
	@echo "BOOTLOADER_S = $(BOOTLOADER_S)"
	@echo "LINKER_SCRIPT = $(LINKER_SCRIPT)"
	@echo "SRCS = $(SRCS)"
	@echo "OBJS = $(OBJS)"
