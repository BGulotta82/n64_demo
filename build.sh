#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"
flatpak-spawn --host distrobox enter n64-dev -T -- bash -lc 'cd "/home/deck/n64_test_project" && export N64_INST=/n64_toolchain && make D=1 "$@"' _ "$@"
