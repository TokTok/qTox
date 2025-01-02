#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2018-2021 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team

# Fail out on error
set -exuo pipefail

# use multiple cores when building
export MAKEFLAGS="-j$(nproc)"
FLATPAK_DESCRIPTOR=$(dirname "$(realpath "$0")")/com.github.TokTok.qTox.json

# If $FLATPAK_BUILD is set, use it as the build directory
if [ -n "${FLATPAK_BUILD:-}" ]; then
  mkdir -p "$FLATPAK_BUILD"
  cd "$FLATPAK_BUILD"
fi

# Build the qTox flatpak
flatpak-builder --ccache --disable-rofiles-fuse --install-deps-from=flathub --force-clean --repo=qtox-repo _build-flatpak "$FLATPAK_DESCRIPTOR"

# Create a bundle for distribution
flatpak build-bundle qtox-repo qtox.flatpak com.github.TokTok.qTox

# If $FLATPAK_BUILD is set, copy the bundle to the build directory
if [ -n "${FLATPAK_BUILD:-}" ]; then
  cp qtox.flatpak "/qtox"
fi

rm -f .flatpak-builder/cache/.lock
