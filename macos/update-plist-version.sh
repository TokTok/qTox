#!/usr/bin/env bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# script to change qTox version in `Info.plist` file to the supplied one
#
# NOTE: it checkouts the files before appending a version to them!
#
# requires:
#  * correctly formatted `Info.plist file in working dir
#  * GNU sed

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot

set -euo pipefail

readonly VERSION_PATTERN='[0-9]+\.[0-9]+\.[0-9]+(-rc\.[0-9]+)?'

# update version in `Info.plist` file to supplied one after the right lines
update_version() {
  local vars=(
    '	<key>CFBundleShortVersionString</key>'
    '	<key>CFBundleVersion</key>'
  )

  for v in "${vars[@]}"; do
    sed -i -r "\\R$v\$R,+1 s,(<string>)$VERSION_PATTERN(</string>)$,\\1$@\\3," \
      "./Info.plist"
  done
}

# exit if supplied arg is not a version
is_version() {
  if [[ ! $@ =~ $VERSION_PATTERN ]]; then
    echo "Not a version: $@"
    exit 1
  fi
}

main() {
  is_version "$@"

  update_version "$@"
}
main "$@"
