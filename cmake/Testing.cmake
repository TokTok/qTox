# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2019 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team.

################################################################################
#
# :: Testing
#
################################################################################

enable_testing()

function(auto_test subsystem module extra_res extra_libs)
  add_executable(test_${module}
    test/${subsystem}/${module}_test.cpp ${extra_res})
  target_link_libraries(test_${module}
    ${BINARY_NAME}_static
    ${CHECK_LIBRARIES}
    Qt6::Test
    ${extra_libs})
  if(QT_FEATURE_static)
    target_link_libraries(test_${module}
      Qt6::QOffscreenIntegrationPlugin)
  endif()
  add_test(
    NAME test_${module}
    COMMAND ${TEST_CROSSCOMPILING_EMULATOR} test_${module})
  # don't require a display server for GUI tests
  set_tests_properties(test_${module} PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

add_subdirectory(test/mock)
add_subdirectory(test/dbutility)

set(TEST_RESOURCES test/resources/test_data.qrc ${${BINARY_NAME}_RESOURCES})

# keep-sorted start
auto_test(chatlog chatlinestorage "" "")
auto_test(chatlog chatwidget "" "")
auto_test(chatlog text "" "")
auto_test(chatlog textformatter "" "")
auto_test(core chatid "" "")
auto_test(core core "${${BINARY_NAME}_RESOURCES}" "mock_library")
auto_test(core fileprogress "" "")
auto_test(core toxfile "" "")
auto_test(core toxid "" "")
auto_test(core toxstring "" "")
auto_test(model chathistory "" "")
auto_test(model conferencemessagedispatcher "" "mock_library")
auto_test(model exiftransform "" "")
auto_test(model friendlistmanager "" "")
auto_test(model friendmessagedispatcher "" "")
auto_test(model messageprocessor "" "")
auto_test(model notificationgenerator "" "mock_library")
auto_test(model sessionchatlog "" "")
auto_test(net bsu "${${BINARY_NAME}_RESOURCES}" "") # needs nodes list
auto_test(net updateversion "" "")
auto_test(persistence dbschema "" "dbutility_library")
auto_test(persistence/dbupgrade dbTo11 "" "dbutility_library")
auto_test(persistence offlinemsgengine "" "")
auto_test(persistence paths "" "")
auto_test(persistence settings "" "")
auto_test(persistence settingsserializer "" "")
if("EmojiOne" IN_LIST SMILEY_PACKS)
  auto_test(persistence smileypack "${SMILEY_RESOURCES}" "") # needs emojione
endif()
auto_test(video videomode "" "")
auto_test(video videoframe "" "${LIBAVUTIL_LIBRARIES}")
auto_test(widget filesform "" "")
auto_test(widget/form/settings generalform "" "")
auto_test(widget/tool identicon "" "")
# keep-sorted end

# GUI tests, run only when requested using the offscreen platform.
# These are heavily dependent on specific Qt versions to be pixel-perfect, so
# if they fail with new versions, it's not necessarily a bug.
# TODO(iphydf): Add an easier way to regenerate golden images.
if(GUI_TESTS)
  auto_test(widget loginscreen "${TEST_RESOURCES}" "")
endif()

if(UNIX)
  auto_test(platform posixsignalnotifier "" "")
  auto_test(platform stacktrace "" "")
endif()
