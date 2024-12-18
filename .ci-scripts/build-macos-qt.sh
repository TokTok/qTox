#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2022 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

set -euo pipefail

readonly SCRIPT_DIR="$(dirname "$(realpath "$0")")"
. "$SCRIPT_DIR/macos_install_deps.sh"

# Needs openssl from build-macos-deps.sh to already have been installed.
install_deps \
  build_qt_macos.sh
