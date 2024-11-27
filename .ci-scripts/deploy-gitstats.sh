#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2016-2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team

cd "$GITSTATS_DIR"
COMMIT=$(git describe)

git init --quiet
git config user.name "qTox bot"
git config user.email "qTox@users.noreply.github.com"

git add .
git commit --quiet -m "Deploy to GH pages from commit: $COMMIT"

echo "Pushing to GH pages..."
touch /tmp/access_key
chmod 600 /tmp/access_key
echo "$access_key" > /tmp/access_key
GIT_SSH_COMMAND="ssh -i /tmp/access_key" git push --force --quiet "git@github.com:qTox/gitstats.git" master:gh-pages
