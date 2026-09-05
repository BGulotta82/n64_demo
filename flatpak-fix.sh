#!/bin/bash
# Reverts to a clean pipe that doesn't pass unrecognized arguments to Podman
flatpak-spawn --host podman exec -i n64-dev mips64-elf-gdb "$@"
