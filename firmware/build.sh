#!/usr/bin/env bash
# Build and flash the DR-VF-01 firmware for the Nordic Thingy:91 X.
#
# Why this script exists: Zephyr does not support spaces anywhere in the
# application path (it splits CONF_FILE on spaces). The repository folder name
# contains one, so the build goes through a space-free symlink to firmware/.
# Remove the symlink indirection once the repository lives in a space-free path.
#
# Usage:
#   ./build.sh [build]        build (pristine only when the configuration changed)
#   ./build.sh flash          flash the normal build over USB (MCUboot)
#   ./build.sh bench          build with the geofence timing benchmark
#   ./build.sh flash-bench    flash the benchmark build
#   ./build.sh cold           build that deletes the stored GNSS data at every boot
#   ./build.sh flash-cold     flash the cold-start build
#   ./build.sh clean          remove every build directory
#
# The bench and cold builds use their own build directories, so they never
# overwrite the normal build. Environment overrides: BOARD, BUILD_DIR,
# LINK_PATH, NCS_SHELL.
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

# Build a variant (a Kconfig option turned on) in its own build directory.
# The sysbuild image is named after the link's directory name.
# ncs-shell.sh changes into the NCS workspace, so every path must be absolute.
build_variant() {
	local name="$1" option="$2"

	"$NCS_SHELL" west build -p auto -b "$BOARD" "$LINK_PATH" -d "${BUILD_DIR}_${name}" -- \
		"-D$(basename "$LINK_PATH")_CONFIG_${option}=y"
	echo "Image: ${BUILD_DIR}_${name}/dfu_application.zip"
}

# Flash the image of a build directory.
flash_dir() {
	local dir="$1"

	[ -f "$dir/dfu_application.zip" ] || {
		echo "error: no image in $dir, build it first" >&2
		exit 1
	}
	"$NCS_SHELL" west thingy91x-dfu --chip nrf91 --image "$dir/dfu_application.zip"
}

case "${1:-build}" in
	build)
		"$NCS_SHELL" west build -p auto -b "$BOARD" "$LINK_PATH" -d "$BUILD_DIR"
		echo "Image: $BUILD_DIR/dfu_application.zip"
		;;
	flash)
		flash_dir "$BUILD_DIR"
		;;
	bench)
		build_variant bench DRVF_GEOFENCE_BENCHMARK
		;;
	flash-bench)
		flash_dir "${BUILD_DIR}_bench"
		;;
	cold)
		build_variant cold DRVF_GNSS_COLD_START
		;;
	flash-cold)
		flash_dir "${BUILD_DIR}_cold"
		;;
	clean)
		rm -rf "$BUILD_DIR" "${BUILD_DIR}_bench" "${BUILD_DIR}_cold"
		echo "Removed the build directories of $BUILD_DIR"
		;;
	*)
		echo "usage: $0 [build|flash|bench|flash-bench|cold|flash-cold|clean]" >&2
		exit 2
		;;
esac
