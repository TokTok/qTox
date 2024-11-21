#!/bin/bash

# Usage: ./build.sh --arch [armv7a|arm64-v8a|x86|x86_64] --build-type [Debug|Release]

set -e

for i in "$@"; do
  case $i in
    --arch=*)
      ARCH="${i#*=}"
      shift
      ;;
    --build-type=*)
      BUILD_TYPE="${i#*=}"
      shift
      ;;
    *) ;;

  esac
done

echo "Building $BUILD_TYPE build for $ARCH"

export QT_ANDROID_KEYSTORE_PATH="$PWD/android/keystore.jks"
export QT_ANDROID_KEYSTORE_ALIAS=mydomain
export QT_ANDROID_KEYSTORE_STORE_PASS=aoeuaoeu
export QT_ANDROID_KEYSTORE_KEY_PASS=aoeuaoeu

/work/android/qt/bin/qt-cmake \
  -DCMAKE_PREFIX_PATH=/work/android/qt/lib/cmake \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DQT_HOST_PATH=/usr \
  -DQT_HOST_PATH_CMAKE_DIR=/usr/lib/cmake \
  -DQT_ANDROID_SIGN_APK=ON \
  -DSPELL_CHECK=OFF \
  -DDESKTOP_NOTIFICATIONS=OFF \
  -DUPDATE_CHECK=ON \
  -DSTRICT_OPTIONS=ON \
  -DANDROID_PLATFORM=23 \
  -DBUILD_TESTING=OFF \
  -GNinja \
  -B_build \
  -H.

cmake --build _build --target apk
