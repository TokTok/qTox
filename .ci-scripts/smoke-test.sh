#!/usr/bin/env bash

set -eux -o pipefail

# Release tags *or* master builds where the current commit is tagged
# with 'v.*' are considered stable.
if [ -n "$(echo "${GITHUB_REF:-}" | grep -o 'refs/tags/v.*')" ] ||
  [ -n "$(git tag --points-at HEAD | grep '^v.*')" ]; then
  REGEX="qTox v.* (stable)"
else
  REGEX="qTox v.* (unstable)"
fi

if [ -n "${GITHUB_TOKEN:-}" ]; then
  GITHUB_AUTH=(-H "Authorization: token $GITHUB_TOKEN")
else
  GITHUB_AUTH=()
fi

VERSION=$(curl \
  -H "Accept: application/vnd.github+json" \
  "${GITHUB_AUTH[@]}" \
  -L \
  https://api.github.com/repos/TokTok/qTox/releases/latest |
  grep '"tag_name": "v.*"' |
  grep -o 'v[^"]*')

"$@" --version | grep "$REGEX" || ("$@" --version && false)
"$@" --update-check | grep "^Latest version: $VERSION" || ("$@" --update-check && false)

# If QTOX_SCREENSHOT isn't set, don't take a screenshot.
if [ -z "${QTOX_SCREENSHOT:-}" ]; then
  exit 0
fi

# Otherwise, run the application, wait 10 seconds for it to start up, then send
# SIGUSR1 to take a screenshot.

set -m # Enable job control.

mkdir -p "$HOME/Pictures" # For the screenshot.
"$@" --portable "$PWD/test/resources/profile" --profile "qtox-test-user" 2>&1 | tee qtox.log &
sleep 10

# We need to get the real PID of the qtox process, which is a descendant of %1.
# %1 may be xvfb-run, wrapping bash, running AppRun, wrapping musl's ld.so.
QTOX_PID="$(grep -o ' : Debug: Process ID: [0-9]*' qtox.log | grep -o '[0-9]*')"
kill -USR1 "$QTOX_PID"

# Wait for another second to make sure the screenshot is taken.
sleep 1

# Kill the application with SIGINT.
kill -INT "$QTOX_PID"

# Wait for the application to exit gracefully. We use pgrep above, in case qtox
# is running inside xvfb-run. We use %1 here for job control (the & above).
fg %1

mv "$HOME/Pictures/qtox-screenshot.png" "$QTOX_SCREENSHOT"
