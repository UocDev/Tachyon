#!/usr/bin/env bash
set -euo pipefail

dnf -y update

dnf -y group install \
    development-tools \
    c-development

dnf -y install \
    gcc \
    gcc-c++ \
    make \
    git \
    wget \
    curl \
    flex \
    bison \
    texinfo \
    gmp-devel \
    mpfr-devel \
    libmpc-devel \
    zlib-devel \
    xz \
    tar \
    patch \
    grub2-tools \
    grub2-tools-extra \
    grub2-pc \
    xorriso \
    mtools \
    dosfstools \
    gcc-x86_64-linux-gnu \
    gcc-c++-x86_64-linux-gnu

dnf clean all

x86_64-linux-gnu-gcc -v
x86_64-linux-gnu-gcc --version
