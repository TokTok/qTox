#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2022 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

set -euo pipefail

if [ ! -d "$SCRIPT_DIR/dockerfiles" ]; then
  git clone --depth=1 https://github.com/TokTok/dockerfiles "$SCRIPT_DIR/dockerfiles"
fi

ARCH="$1"

install_deps() {
  for dep in "$@"; do
    mkdir -p "external/$dep"
    pushd "external/$dep"
    if [ -f "$SCRIPT_DIR/dockerfiles/qtox/build_${dep}_macos.sh" ]; then
      SCRIPT="$SCRIPT_DIR/dockerfiles/qtox/build_${dep}_macos.sh"
    else
      SCRIPT="$SCRIPT_DIR/dockerfiles/qtox/build_$dep.sh"
    fi
    "$SCRIPT" --arch "macos-$ARCH" --libtype "static"
    popd
    rm -rf "external/$dep"
  done
  rmdir external
}
