#!/bin/sh

set -eux

cmake \
  -DCMAKE_TOOLCHAIN_FILE=linux/static-toolchain.cmake \
  -DCMAKE_PREFIX_PATH=/sysroot/opt/qt/lib/cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DSPELL_CHECK=OFF \
  -DUPDATE_CHECK=ON \
  -DSTRICT_OPTIONS=ON \
  -DBUILD_TESTING=OFF \
  -GNinja \
  -B_build \
  -H.

cmake --build _build -- -v

ls -lh _build/qtox
ldd _build/qtox
export QT_QPA_PLATFORM=offscreen
_build/qtox --help
