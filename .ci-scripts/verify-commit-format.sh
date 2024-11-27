#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# Fail out on error
set -eu -o pipefail

# Verify commit messages
readarray -t COMMITS <<<$(curl -s "$GITHUB_CONTEXT" | jq -r '.[0,-1].sha')
COMMIT_RANGE="${COMMITS[1]}~1..${COMMITS[0]}"
bash ./verify-commit-messages.sh "$COMMIT_RANGE"
