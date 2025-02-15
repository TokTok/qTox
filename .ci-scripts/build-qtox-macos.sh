#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2021 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team

# Fail out on error
set -eux -o pipefail

MACOS_BUILD_TYPE="${1:-dist}"
MACOS_ARCH="${2:-arm64}"
MACOS_VERSION="${3:-12.0}"

readonly BIN_NAME="qTox-$MACOS_ARCH-$MACOS_VERSION.dmg"

SCRIPT_DIR=$(dirname "$(realpath "$0")")
if [ ! -d "$SCRIPT_DIR/dockerfiles" ]; then
  git clone --depth=1 https://github.com/TokTok/dockerfiles "$SCRIPT_DIR/dockerfiles"
fi

DEP_PREFIX="$SCRIPT_DIR/dockerfiles/local-deps"

if [ "$MACOS_BUILD_TYPE" == "user" ]; then
  CMAKE=cmake
  PREFIX_PATH="$(brew --prefix qt@6)"
elif [ "$MACOS_BUILD_TYPE" == "dist" ]; then
  CMAKE="$DEP_PREFIX/qt/bin/qt-cmake"
  PREFIX_PATH="$DEP_PREFIX;$(brew --prefix qt@6)"
else
  echo "Unknown arg $MACOS_BUILD_TYPE"
  exit 1
fi

build_qtox() {
  ccache --zero-stats

  # Explicitly include with -isystem to avoid warnings from system headers.
  # CMake will use -I instead of -isystem, so we need to set it manually.
  #
  # UBSAN is OFF for now, because it seems to crash inside Qt.
  # https://github.com/TokTok/qTox/issues/474
  "$CMAKE" \
    -DCMAKE_CXX_FLAGS="-isystem/usr/local/include" \
    -DUBSAN=OFF \
    -DUPDATE_CHECK=ON \
    -DSPELL_CHECK=OFF \
    -DSTRICT_OPTIONS=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOS_VERSION" \
    -DCMAKE_PREFIX_PATH="$PREFIX_PATH" \
    -GNinja \
    -B_build \
    .
  cmake --build _build
  ctest --output-on-failure --parallel "$(sysctl -n hw.ncpu)" --test-dir _build
  cmake --install _build
  cp _build/qTox.dmg "$BIN_NAME"

  ccache --show-stats
}

check() {
  if [[ ! -s "$BIN_NAME" ]]; then
    echo "There's no $BIN_NAME!"
    exit 1
  fi
}

make_hash() {
  shasum -a 256 "$BIN_NAME" >"$BIN_NAME".sha256
}

main() {
  build_qtox
  check
  make_hash
}
main
