# SPDX-License-Identifier: GPL-3.0-or-later Copyright © 2022 by The qTox Project
# Contributors Copyright © 2024 The TokTok team.

add_library(
  mock_library STATIC
  include/mock/mockcoreidhandler.h
  src/mockcoreidhandler.cpp
  include/mock/mockconferencequery.h
  src/mockconferencequery.cpp
  include/mock/mockcoresettings.h
  src/mockcoresettings.cpp
  include/mock/mockbootstraplistgenerator.h
  src/mockbootstraplistgenerator.cpp)

target_include_directories(mock_library PUBLIC include/)
target_link_libraries(mock_library util_library Qt6::Core Qt6::Network Qt6::Gui
                      qtox::warnings)
