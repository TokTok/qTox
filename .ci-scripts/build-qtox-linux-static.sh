#!/bin/sh

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2024 The TokTok team

set -eux

cmake \
  -DCMAKE_TOOLCHAIN_FILE=/sysroot/static-toolchain.cmake \
  -DCMAKE_PREFIX_PATH=/sysroot/opt/qt/lib/cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DSPELL_CHECK=OFF \
  -DUPDATE_CHECK=ON \
  -DSTRICT_OPTIONS=ON \
  -DBUILD_TESTING=OFF \
  -DFULLY_STATIC=ON \
  -GNinja \
  -B_build-static \
  -H.

cmake --build _build-static

ls -lh _build-static/qtox
file _build-static/qtox
QT_QPA_PLATFORM=offscreen _build-static/qtox --help
