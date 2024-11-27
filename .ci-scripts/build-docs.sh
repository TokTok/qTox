#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# Fail out on error
set -eu -o pipefail

# Obtain doxygen and its deps
sudo apt-get update -qq
sudo apt-get install doxygen graphviz

GIT_DESC=$(git describe --tags 2>/dev/null)
GIT_CHASH=$(git rev-parse HEAD)

# Append git version to doxygen version string
echo "PROJECT_NUMBER = \"Version: $GIT_DESC | Commit: $GIT_CHASH\"" >> "$DOXYGEN_CONFIG_FILE"

# Generate documentation
echo "Generating documentation..."
echo

doxygen "$DOXYGEN_CONFIG_FILE"
