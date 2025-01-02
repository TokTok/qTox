#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2019 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team

# script to change versions in the files for macOS and windows "packages"
#
# it should be run before releasing a new version
#
# NOTE: it checkouts the files before appending a version to them!
#
# requires:
#  * GNU sed

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot

set -eu -o pipefail

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly BASE_DIR="$SCRIPT_DIR/../"
readonly VERSION_PATTERN='[0-9]+\.[0-9]+\.[0-9]+(-rc\.[0-9]+)?'

update_windows() {
  (
    cd "$BASE_DIR/windows"
    ./qtox-nsi-version.sh "$@"
  )
}

update_macos() {
  (
    cd "$BASE_DIR/macos"
    ./update-plist-version.sh "$@"
  )
}

update_readme() {
  cd "$BASE_DIR"
  sed -ri "s|(github.com/TokTok/qTox/releases/download/v)$VERSION_PATTERN|\1$@|g" README.md
  # for flatpak and AppImage
  sed -ri "s|(github.com/TokTok/qTox/releases/download/v$VERSION_PATTERN/qTox-v)$VERSION_PATTERN|\1$@|g" README.md
}

update_appdata() {
  cd "$BASE_DIR"/res/
  local isodate="$(date --iso-8601)"
  sed -ri "s|(<release version=\")$VERSION_PATTERN|\1$@|g" io.github.TokTok.qTox.appdata.xml
  sed -ri "s|(<release version=\"$VERSION_PATTERN\" date=\").{10}\"|\1$isodate\"|g" io.github.TokTok.qTox.appdata.xml
}

# exit if supplied arg is not a version
is_version() {
  if [[ ! $@ =~ $VERSION_PATTERN ]]; then
    echo "Not a version: $@"
    echo "Must match: $VERSION_PATTERN"
    exit 1
  fi
}

main() {
  is_version "$@"

  # macOS cannot into proper sed
  if [[ ! "$OSTYPE" == "darwin"* ]]; then
    update_macos "$@"
    update_windows "$@"
    update_readme "$@"
    update_appdata "$@"
  else
    # TODO: actually check whether there is a GNU sed on macOS
    echo "macOS' sed not supported. Get a proper one."
  fi
}
main "$@"
