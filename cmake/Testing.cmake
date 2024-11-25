# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team.

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
    ${PROJECT_NAME}_static
    ${CHECK_LIBRARIES}
    Qt6::Test
    ${extra_libs})
  add_test(
    NAME test_${module}
    COMMAND ${TEST_CROSSCOMPILING_EMULATOR} test_${module})
endfunction()

add_subdirectory(test/mock)
add_subdirectory(test/dbutility)

set(TEST_RESOURCES test/resources/test_data.qrc ${${PROJECT_NAME}_RESOURCES})

# TODO(iphydf): Fix the platform DLL loading so these tests can be re-enabled.
if(NOT WIN32)
  auto_test(core core "${${PROJECT_NAME}_RESOURCES}" "mock_library")
endif()
auto_test(core chatid "" "")
auto_test(core toxid "" "")
auto_test(core toxstring "" "")
auto_test(core fileprogress "" "")
auto_test(chatlog textformatter "" "")
if(NOT WIN32)
  auto_test(net bsu "${${PROJECT_NAME}_RESOURCES}" "") # needs nodes list
endif()
auto_test(chatlog chatlinestorage "" "")
auto_test(persistence paths "" "")
auto_test(persistence dbschema "" "dbutility_library")
auto_test(persistence/dbupgrade dbTo11 "" "dbutility_library")
auto_test(persistence offlinemsgengine "" "")
if(NOT WIN32 AND NOT "${SMILEYS}" STREQUAL "DISABLED")
  auto_test(persistence smileypack "${SMILEY_RESOURCES}" "") # needs emojione
endif()
auto_test(model friendlistmanager "" "")
auto_test(model friendmessagedispatcher "" "")
auto_test(model conferencemessagedispatcher "" "mock_library")
auto_test(model messageprocessor "" "")
auto_test(model sessionchatlog "" "")
auto_test(model exiftransform "" "")
auto_test(model notificationgenerator "" "mock_library")
auto_test(widget filesform "" "")
auto_test(widget/form/settings generalform "" "")

# GUI tests, run only when requested using the offscreen platform.
# These are heavily dependent on specific Qt versions to be pixel-perfect, so
# if they fail with new versions, it's not necessarily a bug.
# TODO(iphydf): Add an easier way to regenerate golden images.
if (GUI_TESTS)
  auto_test(widget loginscreen "${TEST_RESOURCES}" "")
endif()

if (UNIX)
  auto_test(platform posixsignalnotifier "" "")
endif()
