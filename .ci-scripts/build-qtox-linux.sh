#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2021 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

set -euo pipefail

usage() {
  echo "$0 [--minimal|--full] --build-type [Debug|Release] [--with-gui-tests] [--sanitize] [--tidy]"
  echo "Build script to build/test qtox from a CI environment."
  echo "--minimal or --full are required, --build-type is required."
  echo "UndefinedBehaviorSanitizer is always enabled. In Release builds, it is used without additional runtime dependencies."
}

while (($# > 0)); do
  case $1 in
    --minimal)
      MINIMAL=1
      shift
      ;;
    --full)
      MINIMAL=0
      shift
      ;;
    --sanitize)
      SANITIZE=1
      shift
      ;;
    --tidy)
      TIDY=1
      shift
      ;;
    --build-type)
      BUILD_TYPE="$2"
      shift 2
      ;;
    --with-gui-tests)
      GUI_TESTS=1
      shift
      ;;
    --help | -h)
      usage
      exit 1
      ;;
    *)
      echo "Unexpected argument $1"
      usage
      exit 1
      ;;
  esac
done

if [ -z "${MINIMAL+x}" ]; then
  echo "Please build either minimal or full version of qtox"
  usage
  exit 1
fi

if [ -z "${BUILD_TYPE+x}" ]; then
  echo "Please specify build type"
  usage
  exit 1
fi

CMAKE_ARGS=()
if [ ! -z "${SANITIZE+x}" ]; then
  CMAKE_ARGS+=("-DASAN=ON")
fi

if [ ! -z "${GUI_TESTS+x}" ]; then
  CMAKE_ARGS+=("-DGUI_TESTS=ON")
fi

if [ ! -z "${TIDY+x}" ]; then
  export CXX=clang++
  export CC=clang
  CMAKE_ARGS+=("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
fi

SRCDIR=/qtox
export CTEST_OUTPUT_ON_FAILURE=1
export QT_QPA_PLATFORM=offscreen

if [ "$MINIMAL" -eq 1 ]; then
  cmake "$SRCDIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DSTRICT_OPTIONS=ON \
    -DUBSAN=ON \
    -GNinja \
    -DSMILEYS=OFF \
    -DUPDATE_CHECK=OFF \
    -DSPELL_CHECK=OFF \
    "${CMAKE_ARGS[@]}"
else
  cmake "$SRCDIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DSTRICT_OPTIONS=ON \
    -DUBSAN=ON \
    -GNinja \
    -DCODE_COVERAGE=ON \
    -DUPDATE_CHECK=ON \
    "${CMAKE_ARGS[@]}"
fi

cmake --build .

if [ ! -z "${TIDY+x}" ]; then
  run-clang-tidy -header-filter=.* src/ audio/src/ audio/include test/src/ \
    test/include util/src/ util/include/
else
  ctest -j"$(nproc)"
fi
