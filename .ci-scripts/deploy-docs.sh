#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

# Fail out on error
set -eu -o pipefail

# Extract html documentation directory from doxygen configuration
OUTPUT_DIR_CFG=( $(grep 'OUTPUT_DIRECTORY' "$DOXYGEN_CONFIG_FILE") )
HTML_OUTPUT_CFG=( $(grep 'HTML_OUTPUT' "$DOXYGEN_CONFIG_FILE") )

DOCS_DIR="./${OUTPUT_DIR_CFG[2]}/${HTML_OUTPUT_CFG[2]}/"

# Ensure docs exists
if [ ! -d "$DOCS_DIR" ]
then
    echo "Docs deploy failing, no $DOCS_DIR present."
    exit 1
fi

# Obtain git commit hash from HEAD
GIT_CHASH=$(git rev-parse HEAD)

# Push generated doxygen to GitHub pages
cd "$DOCS_DIR"

git init --quiet
git config user.name "qTox bot"
git config user.email "qTox@users.noreply.github.com"

git add .
git commit --quiet -m "Deploy to GH pages from commit: $GIT_CHASH"

echo "Pushing to GH pages..."
touch /tmp/access_key
chmod 600 /tmp/access_key
echo "$access_key" > /tmp/access_key
GIT_SSH_COMMAND="ssh -i /tmp/access_key" git push --force --quiet "git@github.com:qTox/doxygen.git" master:gh-pages
