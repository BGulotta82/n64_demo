#!/usr/bin/env bash
set -e

exec flatpak-spawn --host distrobox enter n64-dev -T -- /n64_toolchain/bin/mips64-elf-gdb "$@"
