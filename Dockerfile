FROM archlinux:latest AS build

RUN pacman-key --init
RUN pacman -Syu --noconfirm gcc nasm make grub xorriso mtools
RUN pacman -Scc --noconfirm

WORKDIR /Build/

COPY . .

RUN make

FROM alpine:latest

RUN apk add --no-cache qemu-system-x86_64

WORKDIR /Tachyoners/
COPY --from=build /Build/release/tachyon.iso ./tachyon.iso

CMD ["qemu-system-x86_64","-cdrom","tachyon.iso","-m","1720","-no-reboot","-nographic"]
