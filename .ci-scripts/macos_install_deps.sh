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
    mkdir -p _build-dep
    pushd _build-dep
    "$SCRIPT_DIR/dockerfiles/qtox/$dep" --arch "macos-$ARCH" --libtype "static"
    popd
    rm -rf _build-dep
  done
}
