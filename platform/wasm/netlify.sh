#!/bin/bash

set -eux -o pipefail

PROJECT=qtox

# Don't build if [netlify skip] is in the commit message.
if git log -1 --pretty="format:%b" | grep '\[netlify skip\]'; then
  echo 'Reusing existing production build.'
  mkdir _site
  cp platform/wasm/_headers _site
  curl -L "https://$PROJECT.netlify.app/qtloader.js" -o "_site/qtloader.js"
  curl -L "https://$PROJECT.netlify.app/qtlogo.svg" -o "_site/qtlogo.svg"
  curl -L "https://$PROJECT.netlify.app/index.html" -o _site/index.html
  curl -L "https://$PROJECT.netlify.app/$PROJECT.js" -o "_site/$PROJECT.js"
  curl -L "https://$PROJECT.netlify.app/$PROJECT.wasm" -o "_site/$PROJECT.wasm"
  exit 0
fi

mkdir -p /opt/buildhome/bin
tar -C /opt/buildhome/bin --strip-components=1 -Jxf \
  <(curl -L https://github.com/ccache/ccache/releases/download/v4.10.2/ccache-4.10.2-linux-x86_64.tar.xz) \
  ccache-4.10.2-linux-x86_64/ccache
tar -C /opt -zxf \
  <(curl -L https://github.com/TokTok/dockerfiles/releases/download/nightly/qtox-wasm-buildhome.tar.gz)

. '/opt/buildhome/emsdk/emsdk_env.sh'
export PATH="/opt/buildhome/bin:$PATH"
export PKG_CONFIG_PATH='/opt/buildhome/lib/pkgconfig'

# Limit ccache to 100MB storage.
ccache -M 100M
ccache --show-stats
ccache --zero-stats

# Set cache directory to $NETLIFY_CACHE_DIR/ccache.
# https://github.com/netlify/build-image/blob/focal/run-build-functions.sh#L11
ccache --set-config=cache_dir=/opt/buildhome/cache/ccache

# Print ccache config.
ccache --show-config

emcmake cmake \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_FIND_ROOT_PATH='/opt/buildhome;/opt/buildhome/qt' \
  -B_build-wasm \
  -H.

cmake --build _build-wasm --parallel "$(nproc)"

ccache --show-stats
du -sh /opt/buildhome/cache/ccache

mkdir _site

cp \
  platform/wasm/_headers \
  _build-wasm/qtloader.js \
  _build-wasm/qtlogo.svg \
  "_build-wasm/$PROJECT.js" \
  "_build-wasm/$PROJECT.wasm" \
  _site
cp "_build-wasm/$PROJECT.html" \
  _site/index.html
