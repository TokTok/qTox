#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2017 Zetok Zalbavar <zetok@openmailbox.org>
# Copyright © 2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# script to change qTox version in `.nsi` files to supplied one
#
# requires:
#  * files `qtox.nsi` and `qtox64.nsi` in working dir
#  * GNU sed

# usage:
#
#   ./$script $version
#
# $version has to be composed of at least one number/dot

set -eu -o pipefail

readonly VERSION_PATTERN='[0-9]+\.[0-9]+\.[0-9]+(-rc\.[0-9]+)?'

# change version in .nsi files in the right line
change_version() {
  for nsi in *.nsi; do
    sed -i -r "/DisplayVersion/ s/\"$VERSION_PATTERN\"$/\"$@\"/" "$nsi"
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

  change_version "$@"
}
main "$@"
