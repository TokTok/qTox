#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later.
# Copyright © 2016-2021 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# Fail out on error
set -eu -o pipefail

readonly BIN_NAME="qTox-$2.dmg"

SCRIPT_DIR=$(dirname "$(realpath "$0")")
if [ ! -d "$SCRIPT_DIR/dockerfiles" ]; then
  git clone --depth=1 https://github.com/TokTok/dockerfiles "$SCRIPT_DIR/dockerfiles"
fi

source "$SCRIPT_DIR/dockerfiles/qtox/build_utils.sh"

# kind of a hack.. but we want to extract DEP_PREFIX from it
parse_arch --arch macos --supported macos --dep macos

if [ "$1" == "user" ]; then
  DEPLOYMENT_TARGET=""
  PREFIX_PATH="$(brew --prefix qt@6)"
elif [ "$1" == "dist" ]; then
  DEPLOYMENT_TARGET="-DCMAKE_OSX_DEPLOYMENT_TARGET=$MACOS_MINIMUM_SUPPORTED_VERSION"
  PREFIX_PATH="$DEP_PREFIX;$(brew --prefix qt@6)"
else
  echo "Unknown arg $1"
  exit 1
fi

build_qtox() {
  # Explicitly include with -isystem to avoid warnings from system headers.
  # CMake will use -I instead of -isystem, so we need to set it manually.
  cmake \
    -DCMAKE_CXX_FLAGS="-isystem/usr/local/include" \
    -DUBSAN=ON \
    -DUPDATE_CHECK=ON \
    -DSPELL_CHECK=OFF \
    -DSTRICT_OPTIONS=ON \
    -DCMAKE_BUILD_TYPE=Release \
    "$DEPLOYMENT_TARGET" \
    -DCMAKE_PREFIX_PATH="$PREFIX_PATH" \
    -GNinja \
    .
  cmake --build .
  ctest --output-on-failure --parallel "$(sysctl -n hw.ncpu)"
  cmake --install .
  cp qTox.dmg "$BIN_NAME"
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
