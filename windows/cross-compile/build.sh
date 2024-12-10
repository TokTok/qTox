#!/usr/bin/env bash

# SPDX-License-Identifier: MIT
# Copyright © 2017-2021 Maxim Biro <nurupo.contributions@gmail.com>
# Copyright © 2024 The TokTok team.

# Known issues:
# - Doesn't build qTox updater, because it wasn't ported to cmake yet and
#   because it requires static Qt, which means we'd need to build Qt twice, and
#   building Qt takes really long time.

set -euo pipefail

RUN_TESTS=0

while (($# > 0)); do
  case $1 in
    --src-dir)
      QTOX_SRC_DIR=$2
      shift 2
      ;;
    --arch)
      ARCH=$2
      shift 2
      ;;
    --run-tests)
      RUN_TESTS=1
      shift
      ;;
    --build-type)
      BUILD_TYPE=$2
      shift 2
      ;;
    *)
      "Unexpected argument $1"
      exit 1
      ;;
  esac
done

# Common directory paths

QTOX_BUILD_DIR="_build-$WINEARCH"
readonly QTOX_BUILD_DIR
QTOX_PREFIX_DIR="$(realpath install-prefix)"
readonly QTOX_PREFIX_DIR
QTOX_PACKAGE_DIR="$(realpath package-prefix)"
readonly QTOX_PACKAGE_DIR

if [ -z "$ARCH" ]; then
  echo "Error: No architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [[ "$ARCH" != "i686" ]] && [[ "$ARCH" != "x86_64" ]]; then
  echo "Error: Incorrect architecture was specified. Please specify either 'i686' or 'x86_64', case sensitive, as the first argument to the script."
  exit 1
fi

if [ -z "$QTOX_SRC_DIR" ]; then
  echo "--src-dir must be specified"
fi

if [ -z "$BUILD_TYPE" ]; then
  echo "Error: No build type was specified. Please specify either 'Release' or 'Debug', case sensitive, as the second argument to the script."
  exit 1
fi

if [[ "$BUILD_TYPE" != "Release" ]] && [[ "$BUILD_TYPE" != "Debug" ]]; then
  echo "Error: Incorrect build type was specified. Please specify either 'Release' or 'Debug', case sensitive, as the second argument to the script."
  exit 1
fi

export PKG_CONFIG_PATH=/windows/lib64/pkgconfig

# Spell check on windows currently not supported, disable
if [[ "$BUILD_TYPE" == "Release" ]]; then
  cmake -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
    -DCMAKE_LIBRARY_PATH=/windows/lib64 \
    -DCMAKE_PREFIX_PATH=/windows \
    -DCMAKE_BUILD_TYPE=Release \
    -DSPELL_CHECK=OFF \
    -DUPDATE_CHECK=ON \
    -DSTRICT_OPTIONS=ON \
    -DTEST_CROSSCOMPILING_EMULATOR=wine \
    -GNinja \
    "-B$QTOX_BUILD_DIR" \
    "$QTOX_SRC_DIR"
elif [[ "$BUILD_TYPE" == "Debug" ]]; then
  cmake -DCMAKE_TOOLCHAIN_FILE=/build/windows-toolchain.cmake \
    -DCMAKE_LIBRARY_PATH=/windows/lib64 \
    -DCMAKE_PREFIX_PATH=/windows \
    -DCMAKE_BUILD_TYPE=Debug \
    -DSPELL_CHECK=OFF \
    -DUPDATE_CHECK=ON \
    -DSTRICT_OPTIONS=ON \
    -DTEST_CROSSCOMPILING_EMULATOR=wine \
    -GNinja \
    -DCMAKE_EXE_LINKER_FLAGS="-mconsole" \
    "-B$QTOX_BUILD_DIR" \
    "$QTOX_SRC_DIR"
fi

cmake --build "$QTOX_BUILD_DIR"

mkdir -p "$QTOX_PREFIX_DIR"
cp "$QTOX_BUILD_DIR/qtox.exe" "$QTOX_PREFIX_DIR"
cp -r /export/* "$QTOX_PREFIX_DIR"

export WINEQT_QPA_PLATFORM='offscreen'
export WINEPREFIX="$PWD/$QTOX_BUILD_DIR/.wine"

# Check if our main binary runs (just to see if any DLL errors happen early on).
# This also initialises the wine directory for tests (avoiding race conditions).
wine "$QTOX_PREFIX_DIR/qtox.exe" --help

# Run tests
set +u
if [[ $RUN_TESTS -ne 0 ]]; then
  # Set up the environment for the tests (not needed for the main binary, where
  # we want to see if the prefix dir has everything we need).
  export WINEQT_PLUGIN_PATH='z:\export'
  export WINEPATH='z:\export;z:\windows\bin'
  ctest --test-dir "$QTOX_BUILD_DIR" --parallel "$(nproc)" --output-on-failure
fi
set -u

# Strip
set +e
if [[ "$BUILD_TYPE" == "Release" ]]; then
  "$ARCH-w64-mingw32-strip" -s "$QTOX_PREFIX_DIR"/*.exe
fi
"$ARCH-w64-mingw32-strip" -s "$QTOX_PREFIX_DIR"/*.dll
"$ARCH-w64-mingw32-strip" -s "$QTOX_PREFIX_DIR"/*/*.dll
set -e

if [[ "$BUILD_TYPE" == "Debug" ]]; then
  cp -r /debug_export/* "$QTOX_PREFIX_DIR"
fi

# Create zip
pushd "$QTOX_PREFIX_DIR"
zip "qtox-$ARCH-$BUILD_TYPE.zip" -r *
popd

# Create installer
if [[ "$BUILD_TYPE" == "Release" ]]; then
  mkdir -p "$QTOX_PACKAGE_DIR"
  pushd "$QTOX_PACKAGE_DIR"
  # The installer creation script expects all the files to be in qtox/*
  mkdir -p qtox
  cp -r "$QTOX_PREFIX_DIR"/* ./qtox
  rm ./qtox/*.zip

  cp -r "$QTOX_SRC_DIR"/windows/* .
  # Select the installer script for the correct architecture
  if [[ "$ARCH" == "i686" ]]; then
    makensis qtox.nsi
  elif [[ "$ARCH" == "x86_64" ]]; then
    makensis qtox64.nsi
  fi

  popd
fi

# dll check
# Create lists of all .exe and .dll files
find "$QTOX_PREFIX_DIR" -iname '*.dll' >dlls
find "$QTOX_PREFIX_DIR" -iname '*.exe' >exes

# Create a list of dlls that are loaded during the runtime (not listed in the PE
# import table, thus ldd doesn't print those)
echo "$QTOX_PREFIX_DIR/iconengines/qsvgicon.dll
$QTOX_PREFIX_DIR/imageformats/qgif.dll
$QTOX_PREFIX_DIR/imageformats/qicns.dll
$QTOX_PREFIX_DIR/imageformats/qico.dll
$QTOX_PREFIX_DIR/imageformats/qjpeg.dll
$QTOX_PREFIX_DIR/imageformats/qsvg.dll
$QTOX_PREFIX_DIR/imageformats/qtga.dll
$QTOX_PREFIX_DIR/imageformats/qtiff.dll
$QTOX_PREFIX_DIR/imageformats/qwbmp.dll
$QTOX_PREFIX_DIR/imageformats/qwebp.dll
$QTOX_PREFIX_DIR/platforms/qdirect2d.dll
$QTOX_PREFIX_DIR/platforms/qminimal.dll
$QTOX_PREFIX_DIR/platforms/qoffscreen.dll
$QTOX_PREFIX_DIR/platforms/qwindows.dll
$QTOX_PREFIX_DIR/tls/qcertonlybackend.dll
$QTOX_PREFIX_DIR/tls/qopensslbackend.dll
$QTOX_PREFIX_DIR/tls/qschannelbackend.dll" >runtime-dlls
if [[ "$ARCH" == "i686" ]]; then
  echo "$QTOX_PREFIX_DIR/libssl-3.dll" >>runtime-dlls
elif [[ "$ARCH" == "x86_64" ]]; then
  echo "$QTOX_PREFIX_DIR/libssl-3-x64.dll" >>runtime-dlls
fi

# Create a tree of all required dlls
# Assumes all .exe files are directly in $QTOX_PREFIX_DIR, not in subdirs
while IFS= read -r line; do
  python3 /usr/local/bin/mingw-ldd.py "$line" --dll-lookup-dirs "$QTOX_PREFIX_DIR" --output-format tree >>dlls-required
done < <(cat exes runtime-dlls)

# Check that no extra dlls get bundled
while IFS= read -r line; do
  if ! grep -q "$line" dlls-required; then
    echo "Error: extra dll included: $line. If this is a mistake and the dll is actually needed (e.g. it's loaded at runtime), please add it to the runtime dll list."
    exit 1
  fi
done <dlls

# Check that no dll is missing. Ignore api-ms-win-*.dll, because they are
# symbolic names pointing at kernel32.dll, user32.dll, etc.
#
# See https://github.com/nurupo/mingw-ldd/blob/master/README.md#api-set-dlls
if grep -q 'not found' dlls-required | grep -q -v 'api-ms-win-'; then
  cat dlls-required
  echo "Error: Missing some dlls."
  exit 1
fi

# Check that OpenAL is bundled. It is availabe from WINE, but not on Windows systems
if grep -q '/opt/wine-stable/lib/wine/i386-windows/openal32.dll' dlls-required; then
  cat dlls-required
  echo "Error: Missing OpenAL."
  exit 1
fi
