#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2022 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"
. "$SCRIPT_DIR/local_install_deps.sh"

install_deps \
  qtbase \
  qttools \
  qtsvg \
  qtimageformats \
  qtremoteobjects \
  qtwayland
