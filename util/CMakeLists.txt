# cmake-format: off
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2020 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team
# cmake-format: on

add_library(
  util_library STATIC
  include/util/algorithm.h
  src/algorithm.cpp
  include/util/laterdeleter.h
  src/laterdeleter.cpp
  include/util/interface.h
  include/util/ranges.h
  src/ranges.cpp
  include/util/strongtype.h
  include/util/display.h
  src/display.cpp
  include/util/toxcoreerrorparser.h
  src/toxcoreerrorparser.cpp)

# We need this directory, and users of our library will need it too
target_include_directories(util_library PUBLIC include/)
target_link_libraries(util_library PRIVATE Qt6::Core)
target_link_libraries(util_library PRIVATE qtox::warnings)
