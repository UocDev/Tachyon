FROM archlinux:latest

RUN pacman-key --init
RUN pacman -Syu --noconfirm git gcc qemu-full nasm make grub xorriso mtools
RUN pacman -Scc --noconfirm

WORKDIR /Doc/

COPY . .

RUN make

CMD ["make", "run"]
