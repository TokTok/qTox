#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2021 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team

# Create lcov report
lcov --ignore-errors inconsistent --directory . --capture --output-file coverage.info
# Filter out system headers and test sources
lcov --ignore-errors inconsistent --remove coverage.info '/usr/*' '*/test/*' '*/*_autogen/*' --output-file coverage.info
