#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# Scripts for generating gitstats in CI
#
# Downloads current git repo, and builds its stats.

# usage:
#   ./$script

# Fail as soon as an error appears
set -eu -o pipefail


make_stats() {
    gitstats \
        -c authors_top=1000 \
        -c max_authors=100000 \
        . \
        "$GITSTATS_DIR"
}

# check if at least something has been generated
verify_exists() {
    [[ -e "$GITSTATS_DIR/index.html" ]]
}


main() {
    make_stats
    verify_exists
}
main
