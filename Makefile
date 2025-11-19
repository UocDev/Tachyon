.PHONY: all boot rootfs pack clean distclean

all: boot rootfs pack

# ---------------------------------------------------------
#  BUILD BOOT (SUBDIRECTORY)
# ---------------------------------------------------------

boot:
	@printf "  BOOT    building boot/\n"
	@$(MAKE) -C boot --no-print-directory

# ---------------------------------------------------------
#  ROOT FILESYSTEM
# ---------------------------------------------------------

rootfs:
	@printf "  ROOTFS  ready\n"

# ---------------------------------------------------------
#  PACK ISO
# ---------------------------------------------------------

pack:
	@printf "  PACK    release/myos.iso\n"
	@mkdir -p release
	@cp boot/myos.iso release/myos.iso

# ---------------------------------------------------------
#  CLEAN
# ---------------------------------------------------------

clean:
	@printf "  CLEAN   boot/\n"
	@$(MAKE) -C boot clean --no-print-directory
	@printf "  CLEAN   release/\n"
	@rm -rf release/*

# ---------------------------------------------------------
#  FULL CLEAN
# ---------------------------------------------------------

distclean: clean
	@printf "  CLEAN   rootFileSystem/\n"
	@rm -rf rootFileSystem/*