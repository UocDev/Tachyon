#!/usr/bin/env bash
set -euo pipefail

make ARCH=i686 V=1
make iso V=1

sleep 5
