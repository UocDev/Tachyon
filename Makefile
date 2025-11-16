.PHONY: all boot rootfs pack clean distclean

all: boot rootfs pack

boot:
	$(MAKE) -C boot

rootfs:
	@echo "[ROOTFS] Root filesystem ready"

pack:
	@mkdir -p release
	cp boot/myos.iso release/myos.iso
	@echo "[PACK] ISO copied to release/myos.iso"

clean:
	$(MAKE) -C boot clean
	rm -rf release/*

distclean: clean
	rm -rf rootFileSystem/*
