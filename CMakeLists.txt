cmake_minimum_required(VERSION 3.10)

# ============================================================
# 1. Penentuan arsitektur dan toolchain (SEBELUM project)
# ============================================================
if(NOT ARCH)
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(ARCH "x86_64")
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i[3-6]86")
        set(ARCH "i386")
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "arm")
        set(ARCH "arm")
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(ARCH "aarch64")
    else()
        message(FATAL_ERROR "Arsitektur host tidak dikenali: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    endif()
endif()
set(ARCH ${ARCH} CACHE STRING "Arsitektur target (x86_64, i386, i686, arm, aarch64)")

# Prefiks cross‑compiler
if(ARCH STREQUAL "x86_64")
    set(CROSS_COMPILE "x86_64-linux-gnu-")
elseif(ARCH STREQUAL "i386" OR ARCH STREQUAL "i686")
    set(CROSS_COMPILE "x86_64-linux-gnu-")   # ubah sesuai distro jika perlu
elseif(ARCH STREQUAL "arm")
    set(CROSS_COMPILE "arm-linux-gnu-")
elseif(ARCH STREQUAL "aarch64")
    set(CROSS_COMPILE "aarch64-linux-gnu-")
else()
    message(FATAL_ERROR "Arsitektur tidak didukung: ${ARCH}")
endif()
set(CROSS_COMPILE ${CROSS_COMPILE} CACHE STRING "Prefiks toolchain")

# Tetapkan compiler dan utilitas SEBELUM project()
set(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_LINKER       ${CROSS_COMPILE}ld)
set(CMAKE_OBJCOPY      ${CROSS_COMPILE}objcopy)
set(CMAKE_OBJDUMP      ${CROSS_COMPILE}objdump)

# ============================================================
# 2. Kunci agar CMake tidak mencoba linking saat pengujian
# ============================================================
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ============================================================
# 3. Project declaration (setelah compiler diatur)
# ============================================================
project(Kernel C CXX ASM)

# ============================================================
# 4. Flag dasar (freestanding)
# ============================================================
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BASE_CFLAGS   "-ffreestanding -nostdlib -Wall -Wextra -pedantic -Wpedantic -pedantic-errors -O0 -g")
set(BASE_CXXFLAGS "-ffreestanding -nostdlib -fno-rtti -fno-exceptions -Wall -Wextra -pedantic -Wpedantic -pedantic-errors -O0 -g")
set(BASE_ASMFLAGS "-ffreestanding -nostdlib -Wall -Wextra -pedantic -Wpedantic -pedantic-errors -O0 -g")
set(BASE_LDFLAGS  "-nostdlib -static -z max-page-size=0x1000")

# ============================================================
# 5. Flag arsitektur spesifik
# ============================================================
if(ARCH STREQUAL "x86_64")
    set(ARCH_CFLAGS   "-m64 -mno-red-zone -fno-stack-protector -mcmodel=large")
    set(ARCH_CXXFLAGS "-m64 -mno-red-zone -fno-stack-protector -mcmodel=large")
    set(ARCH_LDFLAGS  "-m elf_x86_64")
elseif(ARCH STREQUAL "i386" OR ARCH STREQUAL "i686")
    set(ARCH_CFLAGS   "-m32 -mno-red-zone -fno-stack-protector")
    set(ARCH_CXXFLAGS "-m32 -mno-red-zone -fno-stack-protector")
    set(ARCH_LDFLAGS  "-m elf_i386")
elseif(ARCH STREQUAL "arm")
    set(ARCH_CFLAGS   "-marm -mcpu=cortex-a8 -mfloat-abi=soft")
    set(ARCH_CXXFLAGS "-marm -mcpu=cortex-a8 -mfloat-abi=soft")
    set(ARCH_LDFLAGS  "-m armelf")
elseif(ARCH STREQUAL "aarch64")
    set(ARCH_CFLAGS   "-march=armv8-a -mgeneral-regs-only")
    set(ARCH_CXXFLAGS "-march=armv8-a -mgeneral-regs-only")
    set(ARCH_LDFLAGS  "-m aarch64elf")
endif()

set(CMAKE_C_FLAGS   "${BASE_CFLAGS} ${ARCH_CFLAGS}")
set(CMAKE_CXX_FLAGS "${BASE_CXXFLAGS} ${ARCH_CXXFLAGS}")
set(CMAKE_ASM_FLAGS "${BASE_ASMFLAGS} ${ARCH_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${BASE_LDFLAGS} ${ARCH_LDFLAGS}")

# ============================================================
# 6. File sumber dan skrip linker
# ============================================================
set(BOOTLOADER_S "arch/${ARCH}/bootloader_${ARCH}.S")
set(LINKER_SCRIPT "linker/linker_${ARCH}.ld")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${BOOTLOADER_S}")
    message(FATAL_ERROR "File bootloader tidak ditemukan: ${BOOTLOADER_S}")
endif()
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}")
    message(FATAL_ERROR "Skrip linker tidak ditemukan: ${LINKER_SCRIPT}")
endif()

file(GLOB_RECURSE SOURCES_CXX "*.c" "*.cxx")
list(FILTER SOURCES_CXX EXCLUDE REGEX ".*/build/.*")
list(APPEND SOURCES_CXX "${BOOTLOADER_S}")

# ============================================================
# 7. Target kernel.elf
# ============================================================
add_executable(kernel.elf ${SOURCES_CXX})

target_link_options(kernel.elf PRIVATE
    -T "${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}"
)

set_target_properties(kernel.elf PROPERTIES
    LINKER_LANGUAGE C
    EXCLUDE_FROM_ALL FALSE
)

# ============================================================
# 8. Hasil tambahan: kernel.bin dan kernel.asm
# ============================================================
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:kernel.elf> ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin
    DEPENDS kernel.elf
    COMMENT "Membuat kernel.bin (flat binary)"
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel.asm
    COMMAND ${CMAKE_OBJDUMP} -d $<TARGET_FILE:kernel.elf> > ${CMAKE_CURRENT_BINARY_DIR}/kernel.asm
    DEPENDS kernel.elf
    COMMENT "Membuat kernel.asm (disassembly)"
)

add_custom_target(extra_files ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel.bin ${CMAKE_CURRENT_BINARY_DIR}/kernel.asm
)

# ============================================================
# 9. Target ISO (hanya untuk x86)
# ============================================================
set(ISO_DIR "${CMAKE_CURRENT_BINARY_DIR}/iso")
add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ISO_DIR}/boot/grub
    COMMAND cp $<TARGET_FILE:kernel.elf> ${ISO_DIR}/boot/
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/boot/grub.cfg ${ISO_DIR}/boot/grub/grub.cfg
    COMMAND grub-mkrescue -o ${CMAKE_CURRENT_BINARY_DIR}/os.iso ${ISO_DIR}
    DEPENDS kernel.elf
    COMMENT "Membuat ISO image (GRUB)"
)

# ============================================================
# 10. Target QEMU (run & debug)
# ============================================================
set(QEMU_COMMON "-m 512M -monitor none -no-reboot -no-shutdown")
set(QEMU_DEBUG  "-d guest_errors,cpu_reset -D ${CMAKE_CURRENT_BINARY_DIR}/qemu.log")

if(ARCH STREQUAL "arm")
    set(QEMU_CMD qemu-system-arm)
    set(QEMU_OPTS "-M virt -cpu cortex-a15 -kernel $<TARGET_FILE:kernel.elf> -nographic")
    set(QEMU_DEP kernel.elf)
elseif(ARCH STREQUAL "aarch64")
    set(QEMU_CMD qemu-system-aarch64)
    set(QEMU_OPTS "-M virt -cpu cortex-a57 -kernel $<TARGET_FILE:kernel.elf> -nographic")
    set(QEMU_DEP kernel.elf)
else()
    set(QEMU_CMD qemu-system-x86_64)
    set(QEMU_OPTS "-cdrom ${CMAKE_CURRENT_BINARY_DIR}/os.iso -display curses -serial stdio")
    set(QEMU_DEP iso)
    set(QEMU_COMMON "${QEMU_COMMON} -device isa-debug-exit,iobase=0x501,iosize=4")
endif()

add_custom_target(run
    COMMAND ${QEMU_CMD} ${QEMU_COMMON} ${QEMU_OPTS} ${QEMU_DEBUG}
    DEPENDS ${QEMU_DEP}
    COMMENT "Menjalankan kernel di QEMU"
)

add_custom_target(debug
    COMMAND ${QEMU_CMD} ${QEMU_COMMON} ${QEMU_OPTS} ${QEMU_DEBUG} -s -S
    DEPENDS ${QEMU_DEP}
    COMMENT "Menjalankan QEMU dengan GDB stub (port 1234)"
)

# ============================================================
# 11. Target pembersihan
# ============================================================
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Menghapus seluruh direktori build"
)

# ============================================================
# 12. Target informasi & bantuan
# ============================================================
add_custom_target(info
    COMMAND echo "ARCH           = ${ARCH}"
    COMMAND echo "CROSS_COMPILE  = ${CROSS_COMPILE}"
    COMMAND echo "BOOTLOADER_S   = ${BOOTLOADER_S}"
    COMMAND echo "LINKER_SCRIPT  = ${LINKER_SCRIPT}"
    COMMAND echo "SOURCES        = ${SOURCES_CXX}"
    COMMENT "Menampilkan konfigurasi build"
)

add_custom_target(usage
    COMMAND echo "Target utama:"
    COMMAND echo "  all          - bangun kernel.elf (default)"
    COMMAND echo "  iso          - buat ISO (untuk x86)"
    COMMAND echo "  run          - jalankan di QEMU"
    COMMAND echo "  debug        - jalankan dengan GDB stub"
    COMMAND echo "  clean        - hapus file objek (CMake default)"
    COMMAND echo "  distclean    - hapus seluruh direktori build"
    COMMAND echo "  info         - tampilkan konfigurasi"
    COMMAND echo "  usage        - tampilkan bantuan ini"
    COMMENT "Bantuan penggunaan"
)

# ============================================================
# 13. Verbosity (V=1)
# ============================================================
if(DEFINED ENV{V} AND "$ENV{V}" STREQUAL "1")
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
if(DEFINED V AND V EQUAL 1)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
