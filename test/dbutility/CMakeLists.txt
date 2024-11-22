# SPDX-License-Identifier: GPL-3.0-or-later Copyright © 2022 by The qTox Project
# Contributors Copyright © 2024 The TokTok team.

add_library(dbutility_library STATIC include/dbutility/dbutility.h
                                     src/dbutility.cpp)

target_include_directories(dbutility_library PUBLIC include/)
target_link_libraries(dbutility_library Qt6::Core Qt6::Test util_library
                      qtox::warnings)
