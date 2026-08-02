SHELL := /bin/bash
# .ONESHELL:            # Disabled; interferes with verbose/silent output
# .SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables --no-builtin-rules

# Output directories
BUILD_DIR ?= build
OBJ_DIR   := $(BUILD_DIR)/obj

# Verbosity control: V=1 shows full command lines, default shows short logs
V ?= 0

# Architecture selection: default to host architecture, can be overridden
ARCH ?= $(shell uname -m)
ARCH := $(ARCH)

# Toolchain prefix selection based on target architecture
ifeq ($(ARCH), x86_64)
    CROSS_COMPILE ?= x86_64-linux-gnu-
else ifeq ($(ARCH), i386)
    CROSS_COMPILE ?= x86_64-linux-gnu-
else ifeq ($(ARCH), i686)
    CROSS_COMPILE ?= x86_64-linux-gnu-
else ifeq ($(ARCH), arm)
    CROSS_COMPILE ?= arm-linux-gnu-
else ifeq ($(ARCH), aarch64)
    CROSS_COMPILE ?= aarch64-linux-gnu-
else
    $(error Unsupported architecture: $(ARCH))
endif

# Compiler and tool definitions
CC      := $(CROSS_COMPILE)gcc
CXX     := $(CROSS_COMPILE)g++
AS      := $(CROSS_COMPILE)as
LD      := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

# Architecture‑specific bootloader and linker script
BOOTLOADER_S  := boot/bootloader_$(ARCH).S
LINKER_SCRIPT := linker/linker_$(ARCH).ld

ifeq ($(wildcard $(BOOTLOADER_S)),)
    $(error Bootloader file $(BOOTLOADER_S) not found)
endif
ifeq ($(wildcard $(LINKER_SCRIPT)),)
    $(error Linker script $(LINKER_SCRIPT) not found)
endif

# Base flags – freestanding environment, no standard libraries
CFLAGS   := -std=gnu11 -ffreestanding -nostdlib -Wall -Wextra -O2 -g
CXXFLAGS := -std=gnu++17 -ffreestanding -nostdlib -fno-rtti -fno-exceptions -Wall -Wextra -O2 -g
LDFLAGS  := -nostdlib -static -z max-page-size=0x1000

# Architecture‑specific flags
ifeq ($(ARCH), x86_64)
    CFLAGS   += -m64 -mno-red-zone -fno-stack-protector -mcmodel=large
    CXXFLAGS += -m64 -mno-red-zone -fno-stack-protector -mcmodel=large
    LDFLAGS  += -m elf_x86_64
else ifeq ($(ARCH), i386)
    CFLAGS   += -m32 -mno-red-zone -fno-stack-protector
    CXXFLAGS += -m32 -mno-red-zone -fno-stack-protector
    LDFLAGS  += -m elf_i386
else ifeq ($(ARCH), i686)
    # i686 is a 32‑bit x86 variant, same code model as i386
    CFLAGS   += -m32 -mno-red-zone -fno-stack-protector
    CXXFLAGS += -m32 -mno-red-zone -fno-stack-protector
    LDFLAGS  += -m elf_i386
else ifeq ($(ARCH), arm)
    CFLAGS   += -marm -mcpu=cortex-a8 -mfloat-abi=soft
    CXXFLAGS += -marm -mcpu=cortex-a8 -mfloat-abi=soft
    LDFLAGS  += -m armelf
else ifeq ($(ARCH), aarch64)
    # Use general registers only to avoid FP/SIMD in kernel code
    CFLAGS   += -march=armv8-a -mgeneral-regs-only
    CXXFLAGS += -march=armv8-a -mgeneral-regs-only
    # Linker knows target via linker script; no explicit -m needed but set for safety
    LDFLAGS  += -m aarch64elf
endif

# Dependency generation flags
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

# Collect all C and C++ source files (excluding build directory)
SRCS_C_CPP := $(shell find . -type f \( -name "*.c" -o -name "*.cpp" \) -not -path "*/$(BUILD_DIR)/*")
SRCS_C_CPP := $(patsubst ./%,%,$(SRCS_C_CPP))

# Add the architecture‑specific bootloader assembly file
SRCS := $(SRCS_C_CPP) $(BOOTLOADER_S)

# Transform source list into a list of object files under OBJ_DIR
OBJS := $(patsubst %.c,%.o,$(SRCS))
OBJS := $(patsubst %.cpp,%.o,$(OBJS))
OBJS := $(patsubst %.S,%.o,$(OBJS))
OBJS := $(addprefix $(OBJ_DIR)/,$(OBJS))

# VPATH so Make can find sources in their original directories
SRC_DIRS := $(sort $(dir $(SRCS)))
VPATH := $(SRC_DIRS)

# Silent/verbose control
ifeq ($(V),1)
    Q :=                    # Execute commands without @
    MSG := @true            # Suppress short log messages, full commands shown
else
    Q := @                  # Hide commands
    MSG := @echo            # Print short description
endif

# Phony targets
.PHONY: all clean distclean iso run debug submodules info help

# Default target: build both ELF and raw binary
all: $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin

# Link the kernel ELF
$(BUILD_DIR)/kernel.elf: $(OBJS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(MSG) "  LD    $@"
	$(Q)$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $@ $(OBJS)
	$(MSG) "  OBJDUMP $@"
	$(Q)$(OBJDUMP) -d $@ > $(BUILD_DIR)/kernel.asm

# Create a flat binary from the ELF
$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	$(MSG) "  OBJCOPY $@"
	$(Q)$(OBJCOPY) -O binary $< $@

# Compile C sources
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(MSG) "  CC    $<"
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Compile C++ sources
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(MSG) "  CXX   $<"
	$(Q)$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

# Assemble (with C preprocessor) .S files
$(OBJ_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(MSG) "  AS    $<"
	$(Q)$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Include auto‑generated dependency files
DEPS := $(OBJS:.o=.d)
-include $(DEPS)

# Build submodules if any
SUBMODULES := $(shell find . -mindepth 2 -maxdepth 2 -type f -name "Makefile" -printf "%h\n" | sort -u)

submodules:
	@for dir in $(SUBMODULES); do \
		$(MAKE) -C $$dir; \
	done

# ISO image creation (Multiboot2 via GRUB)
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
	$(MSG) "  ISO   $@"
	$(Q)cp $(BUILD_DIR)/kernel.elf $(ISO_DIR)/boot/
	$(Q)grub2-mkrescue -o $(BUILD_DIR)/os.iso $(ISO_DIR)

# QEMU emulation
# ============================================================================
# QEMU Configuration
# ============================================================================

# Common QEMU options
QEMU_COMMON_OPTS := \
    -m 1G \
    -monitor none \
    -no-reboot \
    -no-shutdown

# Debug options
QEMU_DEBUG_OPTS := \
    -d guest_errors,cpu_reset \
    -D $(BUILD_DIR)/qemu.log

ifeq ($(ARCH),arm)
    QEMU := qemu-system-arm
    QEMU_OPTS := \
        $(QEMU_COMMON_OPTS) \
        -M virt \
        -cpu cortex-a15 \
        -kernel $(BUILD_DIR)/kernel.elf \
        -nographic \
        $(QEMU_DEBUG_OPTS)

else ifeq ($(ARCH),aarch64)
    QEMU := qemu-system-aarch64
    QEMU_OPTS := \
        $(QEMU_COMMON_OPTS) \
        -M virt \
        -cpu cortex-a57 \
        -kernel $(BUILD_DIR)/kernel.elf \
        -nographic \
        $(QEMU_DEBUG_OPTS)

else
    # x86 (i386, i686, x86_64)
    QEMU := qemu-system-x86_64
    QEMU_OPTS := \
        $(QEMU_COMMON_OPTS) \
        -cdrom $(BUILD_DIR)/os.iso \
        -display none \
        -serial stdio \
        $(QEMU_DEBUG_OPTS)
endif

# Run the kernel normally
run: iso
	$(MSG) "  QEMU  $(QEMU) $(QEMU_OPTS)"
	$(Q)$(QEMU) $(QEMU_OPTS)

# Run the kernel with a GDB stub (listening on port 1234)
debug: iso
	$(MSG) "  QEMU (debug) $(QEMU) $(QEMU_OPTS) -s -S"
	$(Q)$(QEMU) $(QEMU_OPTS) -s -S

# Clean build artifacts
clean:
	$(MSG) "  CLEAN"
	$(Q)rm -rf $(BUILD_DIR)

# Deep clean: remove build directory and all dependency files
distclean: clean
	$(Q)find . -name "*.d" -delete

# Print current build configuration
info:
	@echo "ARCH           = $(ARCH)"
	@echo "CROSS_COMPILE  = $(CROSS_COMPILE)"
	@echo "BOOTLOADER_S   = $(BOOTLOADER_S)"
	@echo "LINKER_SCRIPT  = $(LINKER_SCRIPT)"
	@echo "SRCS           = $(SRCS)"
	@echo "OBJS           = $(OBJS)"

# Show usage help
help:
	@echo "Usage: make [target] [ARCH=arch] [V=1]"
	@echo ""
	@echo "Common targets:"
	@echo "  all        - Build kernel.elf and kernel.bin"
	@echo "  iso        - Create a bootable ISO image (via GRUB)"
	@echo "  run        - Build and run the kernel in QEMU"
	@echo "  debug      - Build and run with GDB stub (port 1234)"
	@echo "  clean      - Remove the build directory"
	@echo "  distclean  - Clean and delete all dependency files"
	@echo "  submodules - Build any external submodules"
	@echo "  info       - Show configuration details"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  ARCH       - Target architecture (x86_64, i386, i686, arm, aarch64)"
	@echo "  V=1        - Enable verbose output (show full commands)"
