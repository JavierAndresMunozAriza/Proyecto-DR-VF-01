#!/usr/bin/env bash
# Build and flash the DR-VF-01 firmware for the Nordic Thingy:91 X.
#
# Why this script exists: Zephyr does not support spaces anywhere in the
# application path (it splits CONF_FILE on spaces). The repository folder name
# contains one, so the build goes through a space-free symlink to firmware/.
# Remove the symlink indirection once the repository lives in a space-free path.
#
# Usage:
#   ./build.sh [build]   build (pristine only when the configuration changed)
#   ./build.sh clean     remove the build directory
#   ./build.sh flash     flash build/dfu_application.zip over USB (MCUboot)
#
# Environment overrides: BOARD, BUILD_DIR, LINK_PATH, NCS_SHELL.
set -euo pipefail

FIRMWARE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BOARD="${BOARD:-thingy91x/nrf9151/ns}"
BUILD_DIR="${BUILD_DIR:-$HOME/.cache/dr-vf-01/build}"
LINK_PATH="${LINK_PATH:-$HOME/.cache/dr-vf-01/fw}"
NCS_SHELL="${NCS_SHELL:-$HOME/nordic/ncs-shell.sh}"

# Same trap as Zephyr: refuse a build directory path with spaces.
case "$BUILD_DIR$LINK_PATH" in
	*" "*) echo "error: BUILD_DIR and LINK_PATH must not contain spaces" >&2; exit 1 ;;
esac

# Refresh the symlink so it always points at this checkout.
mkdir -p "$(dirname "$LINK_PATH")"
ln -sfn "$FIRMWARE_DIR" "$LINK_PATH"

# ncs-shell.sh changes into the NCS workspace, so every path must be absolute.
case "${1:-build}" in
	build)
		"$NCS_SHELL" west build -p auto -b "$BOARD" "$LINK_PATH" -d "$BUILD_DIR"
		echo "Image: $BUILD_DIR/dfu_application.zip"
		;;
	clean)
		rm -rf "$BUILD_DIR"
		echo "Removed $BUILD_DIR"
		;;
	flash)
		[ -f "$BUILD_DIR/dfu_application.zip" ] || {
			echo "error: no image, run ./build.sh first" >&2
			exit 1
		}
		"$NCS_SHELL" west thingy91x-dfu --chip nrf91 \
			--image "$BUILD_DIR/dfu_application.zip"
		;;
	*)
		echo "usage: $0 [build|clean|flash]" >&2
		exit 2
		;;
esac
