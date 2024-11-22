# SPDX-License-Identifier: GPL-3.0-or-later Copyright © 2020 by The qTox Project
# Contributors Copyright © 2024 The TokTok team.

set(SOURCE_FILES
    "include/audio/audio.h"
    "include/audio/iaudiocontrol.h"
    "include/audio/iaudiosettings.h"
    "include/audio/iaudiosink.h"
    "include/audio/iaudiosource.h"
    "src/iaudiosettings.cpp"
    "src/iaudiosink.cpp"
    "src/audio.cpp"
    "src/backend/alsink.cpp"
    "src/backend/alsink.h"
    "src/backend/alsource.cpp"
    "src/backend/alsource.h"
    "src/backend/openal.cpp"
    "src/backend/openal.h")

add_library(audio_library STATIC ${SOURCE_FILES} resources/audio_res.qrc)

# We need this directory, and users of our library will need it too
target_include_directories(audio_library PUBLIC include/)

target_link_libraries(audio_library util_library)
target_link_libraries(audio_library Qt6::Core)
target_link_libraries(audio_library ${OPENAL_LIBRARIES})
target_link_libraries(audio_library qtox::warnings)
