# cmake-format: off
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2019 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team
# cmake-format: on

# ##############################################################################
#
# :: CMake configuration
#
# ##############################################################################

cmake_minimum_required(VERSION 3.16)
cmake_policy(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

option(SMILEYS "Enable smileys support (will greatly increase the binary size)"
       ON)
option(PLATFORM_EXTENSIONS
       "Enable platform specific extensions, requires extra dependencies" ON)
option(UPDATE_CHECK "Enable automatic update check" ON)
option(USE_CCACHE "Use ccache when available" ON)
option(SPELL_CHECK "Enable spellcheck support" ON)
option(ASAN "Compile with AddressSanitizer" OFF)
option(TSAN "Compile with ThreadSanitizer" OFF)
option(UBSAN "Compile with UndefinedBehaviorSanitizer" OFF)
option(STRICT_OPTIONS "Error on compile warning, used by CI" OFF)
option(BUILD_TESTING "Enable tests (turn OFF for static builds)" ON)
option(FULLY_STATIC
       "Produce a fully statically linked qTox binary (experimental)" OFF)
option(GUI_TESTS
       "Enable GUI rendering tests, likely tied to a specific Qt version" OFF)
option(
  REPR_RCC
  "Set the version of the Qt resource file format to 1 to help with reproducible builds"
  OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Debug
      CACHE STRING
            "Options are: None, Debug, Release, RelWithDebInfo, MinSizeRel."
            FORCE)
endif()

set(ENV{PKG_CONFIG_PATH}
    ${CMAKE_SOURCE_DIR}/libs/lib/pkgconfig:/opt/ffmpeg/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}
)

# necessary to find openal-soft on mac os
if(APPLE)
  # The /opt version is needed on arm64, but it doesn't harm on x86_64. The
  # /usr/local version is needed on x86_64, but it doesn't harm on arm64.
  set(ENV{PKG_CONFIG_PATH}
      $ENV{PKG_CONFIG_PATH}:/opt/homebrew/opt/openal-soft/lib/pkgconfig:/usr/local/opt/openal-soft/lib/pkgconfig
  )
endif()

execute_process(
  COMMAND brew --prefix qt@6
  OUTPUT_VARIABLE QT_PREFIX_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

project(
  qTox
  VERSION 1.18.3
  LANGUAGES CXX)
set(PROJECT_ORG "io.github.qtox")

string(TOLOWER ${PROJECT_NAME} BINARY_NAME)

# C++ standard.
set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
endif()

if(ASAN
   OR TSAN
   OR UBSAN)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
endif()

if(ASAN)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

if(TSAN)
  if(ASAN)
    message(FATAL_ERROR "ASAN and TSAN cannot be enabled at the same time")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=thread")
endif()

if(UBSAN)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Add these to common flags (release and debug), because we can use the
    # runtime-less version via -fsanitize-trap=all below.
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fsanitize=undefined,nullability,local-bounds,float-divide-by-zero"
    )
    set(CMAKE_LINKER_FLAGS
        "${CMAKE_LINKER_FLAGS} -fsanitize=undefined,nullability,local-bounds,float-divide-by-zero"
    )
    # In release, reduce attack surface by never calling into a runtime library,
    # and instead just aborting the program on UB.
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fsanitize-trap=all")
    set(CMAKE_LINKER_FLAGS_RELEASE
        "${CMAKE_LINKER_FLAGS_RELEASE} -fsanitize-trap=all")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=undefined")
    # This is the GCC version of -fsanitize-trap=all.
    set(CMAKE_CXX_FLAGS_RELEASE
        "${CMAKE_CXX_FLAGS_RELEASE} -fno-sanitize-recover=all")
  else()
    message(
      FATAL_ERROR
        "UBSAN is not supported by the compiler (${CMAKE_CXX_COMPILER_ID})")
  endif()
endif()

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Run resource compilation automatically
set(CMAKE_AUTORCC ON)

# Set resource compilation options.

# Enable maximum compression.
set(AUTORCC_OPTIONS -compress 9)

if(REPR_RCC)
  # Drop the file modification time of source files from generated files to help
  # with reproducible builds. We do not use QFileInfo.lastModified so this has
  # no unwanted side effects.
  set(AUTORCC_OPTIONS ${AUTORCC_OPTIONS} -format-version 1 --compress-algo zlib)
endif()

# Apply resource compilation options.
set(CMAKE_AUTORCC_OPTIONS ${AUTORCC_OPTIONS})

# Emscripten needs -fexceptions at link time.
if(NOT EMSCRIPTEN)
  # Disable exceptions (Qt doesn't use them, we don't need them).
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
endif()

# Hardening flags (ASLR, warnings, etc)
set(CMAKE_POSITION_INDEPENDENT_CODE True)

if(NOT WIN32 AND NOT HAIKU)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
endif()

if(UNIX AND NOT APPLE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,now")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro")
endif()

add_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
add_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_RESTRICTED_CAST_FROM_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)

# Use ccache when available to speed up builds.
if(CMAKE_CXX_COMPILER_LAUNCHER)
  message(STATUS "Using ${CMAKE_CXX_COMPILER_LAUNCHER} as compiler launcher")
elseif(USE_CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    message(STATUS "Using ccache")
  else()
    message(STATUS "Not using ccache (not found)")
  endif()
else()
  message(
    STATUS
      "ccache disabled; set option USE_CCACHE=ON to use ccache if available")
endif()

# Search for headers in current directory.
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

include(Dependencies)

# ##############################################################################
#
# :: qTox main library sources
#
# ##############################################################################

qt6_wrap_ui(
  ${BINARY_NAME}_FORMS
  src/chatlog/content/filetransferwidget.ui
  src/loginscreen.ui
  src/mainwindow.ui
  src/widget/about/aboutfriendform.ui
  src/widget/form/debug/debuglog.ui
  src/widget/form/debug/debugobjecttree.ui
  src/widget/form/loadhistorydialog.ui
  src/widget/form/profileform.ui
  src/widget/form/removechatdialog.ui
  src/widget/form/searchsettingsform.ui
  src/widget/form/setpassworddialog.ui
  src/widget/form/settings/aboutsettings.ui
  src/widget/form/settings/advancedsettings.ui
  src/widget/form/settings/avform.ui
  src/widget/form/settings/generalsettings.ui
  src/widget/form/settings/privacysettings.ui
  src/widget/form/settings/userinterfacesettings.ui)

set(${BINARY_NAME}_RESOURCES res.qrc)

if(SMILEYS)
  if(NOT SMILEY_PACKS)
    file(
      GLOB SMILEY_QRCS
      RELATIVE ${CMAKE_SOURCE_DIR}
      smileys/*/emoticons.qrc)
    # smileys/$PACK/emoticons.qrc -> $PACK
    set(SMILEY_PACKS "")
    foreach(SMILEY_PACK ${SMILEY_QRCS})
      get_filename_component(SMILEY_PACK ${SMILEY_PACK} DIRECTORY)
      get_filename_component(SMILEY_PACK ${SMILEY_PACK} NAME)
      list(APPEND SMILEY_PACKS ${SMILEY_PACK})
    endforeach()
  endif()
  message(STATUS "Using smiley packs ${SMILEY_PACKS}")

  set(SMILEY_RESOURCES "")
  foreach(SMILEY_PACK ${SMILEY_PACKS})
    set(SMILEY_RESOURCES ${SMILEY_RESOURCES}
                         smileys/${SMILEY_PACK}/emoticons.qrc)
  endforeach()
endif()

set(${BINARY_NAME}_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp
    src/version.h
    src/appmanager.cpp
    src/appmanager.h
    src/friendlist.cpp
    src/friendlist.h
    src/conferencelist.cpp
    src/conferencelist.h
    src/ipc.cpp
    src/ipc.h
    src/nexus.cpp
    src/nexus.h
    src/chatlog/chatlinecontent.cpp
    src/chatlog/chatlinecontent.h
    src/chatlog/chatlinecontentproxy.cpp
    src/chatlog/chatlinecontentproxy.h
    src/chatlog/chatline.cpp
    src/chatlog/chatline.h
    src/chatlog/chatlinestorage.cpp
    src/chatlog/chatlinestorage.h
    src/chatlog/chatwidget.cpp
    src/chatlog/chatwidget.h
    src/chatlog/chatmessage.cpp
    src/chatlog/chatmessage.h
    src/chatlog/content/filetransferwidget.cpp
    src/chatlog/content/filetransferwidget.h
    src/chatlog/content/image.cpp
    src/chatlog/content/image.h
    src/chatlog/content/notificationicon.cpp
    src/chatlog/content/notificationicon.h
    src/chatlog/content/spinner.cpp
    src/chatlog/content/spinner.h
    src/chatlog/content/text.cpp
    src/chatlog/content/text.h
    src/chatlog/content/timestamp.cpp
    src/chatlog/content/timestamp.h
    src/chatlog/content/broken.cpp
    src/chatlog/content/broken.h
    src/chatlog/customtextdocument.cpp
    src/chatlog/customtextdocument.h
    src/chatlog/documentcache.cpp
    src/chatlog/documentcache.h
    src/chatlog/imageloader.cpp
    src/chatlog/imageloader.h
    src/chatlog/pixmapcache.cpp
    src/chatlog/pixmapcache.h
    src/core/toxfileprogress.cpp
    src/core/toxfileprogress.h
    src/chatlog/textformatter.cpp
    src/chatlog/textformatter.h
    src/core/coreav.cpp
    src/core/coreav.h
    src/core/core.cpp
    src/core/corefile.cpp
    src/core/corefile.h
    src/core/core.h
    src/core/dhtserver.cpp
    src/core/dhtserver.h
    src/core/icoresettings.cpp
    src/core/icoresettings.h
    src/core/icorefriendmessagesender.cpp
    src/core/icorefriendmessagesender.h
    src/core/icoreconferencemessagesender.cpp
    src/core/icoreconferencemessagesender.h
    src/core/icoreconferencequery.cpp
    src/core/icoreconferencequery.h
    src/core/icoreidhandler.cpp
    src/core/icoreidhandler.h
    src/core/idebugsettings.cpp
    src/core/idebugsettings.h
    src/core/toxcall.cpp
    src/core/toxcall.h
    src/core/toxencrypt.cpp
    src/core/toxencrypt.h
    src/core/toxfile.cpp
    src/core/toxfile.h
    src/core/toxfilepause.h
    src/core/toxid.cpp
    src/core/toxid.h
    src/core/conferenceid.cpp
    src/core/conferenceid.h
    src/core/toxlogger.cpp
    src/core/toxlogger.h
    src/core/toxoptions.cpp
    src/core/toxoptions.h
    src/core/toxpk.cpp
    src/core/toxpk.h
    src/core/chatid.cpp
    src/core/chatid.h
    src/core/toxstring.cpp
    src/core/toxstring.h
    src/model/about/aboutfriend.cpp
    src/model/about/aboutfriend.h
    src/model/about/iaboutfriend.cpp
    src/model/about/iaboutfriend.h
    src/model/chathistory.cpp
    src/model/chathistory.h
    src/model/chatlogitem.cpp
    src/model/chatlogitem.h
    src/model/chatroom/friendchatroom.cpp
    src/model/chatroom/friendchatroom.h
    src/model/chatroom/conferenceroom.cpp
    src/model/chatroom/conferenceroom.h
    src/model/chat.cpp
    src/model/chat.h
    src/model/debug/debuglogmodel.cpp
    src/model/debug/debuglogmodel.h
    src/model/debug/debugobjecttreemodel.cpp
    src/model/debug/debugobjecttreemodel.h
    src/model/dialogs/idialogs.cpp
    src/model/dialogs/idialogs.h
    src/model/dialogs/idialogsmanager.h
    src/model/dialogs/idialogsmanager.cpp
    src/model/exiftransform.cpp
    src/model/exiftransform.h
    src/model/friendlist/friendlistmanager.cpp
    src/model/friendlist/friendlistmanager.h
    src/model/friendlist/ifriendlistitem.cpp
    src/model/friendlist/ifriendlistitem.h
    src/model/friendmessagedispatcher.cpp
    src/model/friendmessagedispatcher.h
    src/model/friend.cpp
    src/model/friend.h
    src/model/conferenceinvite.cpp
    src/model/conferenceinvite.h
    src/model/conferencemessagedispatcher.cpp
    src/model/conferencemessagedispatcher.h
    src/model/conference.cpp
    src/model/conference.h
    src/model/ibootstraplistgenerator.cpp
    src/model/ibootstraplistgenerator.h
    src/model/ichatlog.h
    src/model/imessagedispatcher.cpp
    src/model/imessagedispatcher.h
    src/model/message.cpp
    src/model/message.h
    src/model/notificationgenerator.cpp
    src/model/notificationgenerator.h
    src/model/profile/iprofileinfo.cpp
    src/model/profile/iprofileinfo.h
    src/model/profile/profileinfo.cpp
    src/model/profile/profileinfo.h
    src/model/sessionchatlog.cpp
    src/model/sessionchatlog.h
    src/model/status.cpp
    src/model/status.h
    src/model/toxclientstandards.h
    src/net/bootstrapnodeupdater.cpp
    src/net/bootstrapnodeupdater.h
    src/net/avatarbroadcaster.cpp
    src/net/avatarbroadcaster.h
    src/net/toxuri.cpp
    src/net/toxuri.h
    src/net/updatecheck.cpp
    src/net/updatecheck.h
    src/net/updateversion.cpp
    src/net/updateversion.h
    src/persistence/db/rawdatabase.cpp
    src/persistence/db/rawdatabase.h
    src/persistence/db/rawdatabaseimpl.cpp
    src/persistence/db/rawdatabaseimpl.h
    src/persistence/db/upgrades/dbupgrader.cpp
    src/persistence/db/upgrades/dbupgrader.h
    src/persistence/db/upgrades/dbto11.h
    src/persistence/db/upgrades/dbto11.cpp
    src/persistence/history.cpp
    src/persistence/history.h
    src/persistence/ifriendsettings.cpp
    src/persistence/ifriendsettings.h
    src/persistence/iconferencesettings.cpp
    src/persistence/iconferencesettings.h
    src/persistence/inotificationsettings.cpp
    src/persistence/inotificationsettings.h
    src/persistence/ismileysettings.cpp
    src/persistence/ismileysettings.h
    src/persistence/offlinemsgengine.cpp
    src/persistence/offlinemsgengine.h
    src/persistence/paths.cpp
    src/persistence/paths.h
    src/persistence/profile.cpp
    src/persistence/profile.h
    src/persistence/profilelocker.cpp
    src/persistence/profilelocker.h
    src/persistence/serialize.cpp
    src/persistence/serialize.h
    src/persistence/settings.cpp
    src/persistence/settings.h
    src/persistence/settingsserializer.cpp
    src/persistence/settingsserializer.h
    src/persistence/globalsettingsupgrader.cpp
    src/persistence/globalsettingsupgrader.h
    src/persistence/personalsettingsupgrader.cpp
    src/persistence/personalsettingsupgrader.h
    src/persistence/smileypack.cpp
    src/persistence/smileypack.h
    src/persistence/toxsave.cpp
    src/persistence/toxsave.h
    src/platform/autorun.h
    src/platform/autorun_osx.cpp
    src/platform/autorun_win.cpp
    src/platform/autorun_xdg.cpp
    src/platform/camera/directshow.cpp
    src/platform/camera/directshow.h
    src/platform/camera/v4l2.cpp
    src/platform/camera/v4l2.h
    src/platform/capslock.h
    src/platform/capslock_osx.cpp
    src/platform/capslock_win.cpp
    src/platform/capslock_x11.cpp
    src/platform/desktop_notifications/desktopnotify.cpp
    src/platform/desktop_notifications/desktopnotify.h
    src/platform/desktop_notifications/desktopnotifybackend_dbus.cpp
    src/platform/desktop_notifications/desktopnotifybackend_noop.cpp
    src/platform/desktop_notifications/desktopnotifybackend.h
    src/platform/posixsignalnotifier.cpp
    src/platform/posixsignalnotifier.h
    src/platform/sandbox.cpp
    src/platform/sandbox.h
    src/platform/sandboxclient.cpp
    src/platform/sandboxclient.h
    src/platform/sandboxserver.cpp
    src/platform/sandboxserver.h
    src/platform/screenshot_dbus.cpp
    src/platform/screenshot.cpp
    src/platform/screenshot.h
    src/platform/stacktrace.cpp
    src/platform/stacktrace.h
    src/platform/timer.h
    src/platform/timer_osx.cpp
    src/platform/timer_win.cpp
    src/platform/timer_x11.cpp
    src/platform/x11_display.cpp
    src/video/cameradevice.cpp
    src/video/cameradevice.h
    src/video/camerasource.cpp
    src/video/camerasource.h
    src/video/corevideosource.cpp
    src/video/corevideosource.h
    src/video/ivideosettings.cpp
    src/video/ivideosettings.h
    src/video/netcamview.cpp
    src/video/netcamview.h
    src/video/scopedavdictionary.cpp
    src/video/scopedavdictionary.h
    src/video/videoframe.cpp
    src/video/videoframe.h
    src/video/videomode.cpp
    src/video/videomode.h
    src/video/videosource.cpp
    src/video/videosource.h
    src/video/videosurface.cpp
    src/video/videosurface.h
    src/widget/about/aboutfriendform.cpp
    src/widget/about/aboutfriendform.h
    src/widget/categorywidget.cpp
    src/widget/categorywidget.h
    src/widget/chatformheader.cpp
    src/widget/chatformheader.h
    src/widget/circlewidget.cpp
    src/widget/circlewidget.h
    src/widget/commondialogs.cpp
    src/widget/commondialogs.h
    src/widget/contentdialog.cpp
    src/widget/contentdialog.h
    src/widget/contentdialogmanager.cpp
    src/widget/contentdialogmanager.h
    src/widget/contentlayout.cpp
    src/widget/contentlayout.h
    src/widget/emoticonswidget.cpp
    src/widget/emoticonswidget.h
    src/widget/flowlayout.cpp
    src/widget/flowlayout.h
    src/widget/imagepreviewwidget.h
    src/widget/imagepreviewwidget.cpp
    src/widget/searchform.cpp
    src/widget/searchform.h
    src/widget/searchtypes.h
    src/widget/form/addfriendform.cpp
    src/widget/form/addfriendform.h
    src/widget/form/chatform.cpp
    src/widget/form/chatform.h
    src/widget/form/debug/debuglog.cpp
    src/widget/form/debug/debuglog.h
    src/widget/form/debug/debugobjecttree.cpp
    src/widget/form/debug/debugobjecttree.h
    src/widget/form/debugwidget.cpp
    src/widget/form/debugwidget.h
    src/widget/form/filesform.cpp
    src/widget/form/filesform.h
    src/widget/form/filetransferlist.h
    src/widget/form/genericchatform.cpp
    src/widget/form/genericchatform.h
    src/widget/form/conferenceform.cpp
    src/widget/form/conferenceform.h
    src/widget/form/conferenceinviteform.cpp
    src/widget/form/conferenceinviteform.h
    src/widget/form/conferenceinvitewidget.cpp
    src/widget/form/conferenceinvitewidget.h
    src/widget/form/loadhistorydialog.cpp
    src/widget/form/loadhistorydialog.h
    src/widget/form/profileform.cpp
    src/widget/form/profileform.h
    src/widget/form/searchsettingsform.cpp
    src/widget/form/searchsettingsform.h
    src/widget/form/setpassworddialog.cpp
    src/widget/form/setpassworddialog.h
    src/widget/form/settings/aboutform.cpp
    src/widget/form/settings/aboutform.h
    src/widget/form/settings/advancedform.cpp
    src/widget/form/settings/advancedform.h
    src/widget/form/settings/avform.cpp
    src/widget/form/settings/avform.h
    src/widget/form/settings/generalform.cpp
    src/widget/form/settings/generalform.h
    src/widget/form/settings/genericsettings.cpp
    src/widget/form/settings/genericsettings.h
    src/widget/form/settings/privacyform.cpp
    src/widget/form/settings/privacyform.h
    src/widget/form/settings/textcompose.cpp
    src/widget/form/settings/textcompose.h
    src/widget/form/settings/userinterfaceform.h
    src/widget/form/settings/userinterfaceform.cpp
    src/widget/form/settings/verticalonlyscroller.cpp
    src/widget/form/settings/verticalonlyscroller.h
    src/widget/form/settingswidget.cpp
    src/widget/form/settingswidget.h
    src/widget/form/tabcompleter.cpp
    src/widget/form/tabcompleter.h
    src/widget/friendlistlayout.cpp
    src/widget/friendlistlayout.h
    src/widget/friendlistwidget.cpp
    src/widget/friendlistwidget.h
    src/widget/friendwidget.cpp
    src/widget/friendwidget.h
    src/widget/genericchatitemlayout.cpp
    src/widget/genericchatitemlayout.h
    src/widget/genericchatitemwidget.cpp
    src/widget/genericchatitemwidget.h
    src/widget/genericchatroomwidget.cpp
    src/widget/genericchatroomwidget.h
    src/widget/conferencewidget.cpp
    src/widget/conferencewidget.h
    src/widget/loginscreen.cpp
    src/widget/loginscreen.h
    src/widget/maskablepixmapwidget.cpp
    src/widget/maskablepixmapwidget.h
    src/widget/notificationedgewidget.cpp
    src/widget/notificationedgewidget.h
    src/widget/notificationscrollarea.cpp
    src/widget/notificationscrollarea.h
    src/widget/passwordedit.cpp
    src/widget/passwordedit.h
    src/widget/popup.cpp
    src/widget/popup.h
    src/widget/qrwidget.cpp
    src/widget/qrwidget.h
    src/widget/splitterrestorer.cpp
    src/widget/splitterrestorer.h
    src/widget/style.cpp
    src/widget/style.h
    src/widget/tool/abstractscreenshotgrabber.cpp
    src/widget/tool/abstractscreenshotgrabber.h
    src/widget/tool/activatedialog.cpp
    src/widget/tool/activatedialog.h
    src/widget/tool/adjustingscrollarea.cpp
    src/widget/tool/adjustingscrollarea.h
    src/widget/tool/imessageboxmanager.h
    src/widget/tool/imessageboxmanager.cpp
    src/widget/tool/messageboxmanager.cpp
    src/widget/tool/messageboxmanager.h
    src/widget/tool/callconfirmwidget.cpp
    src/widget/tool/callconfirmwidget.h
    src/widget/tool/chattextedit.cpp
    src/widget/tool/chattextedit.h
    src/widget/tool/croppinglabel.cpp
    src/widget/tool/croppinglabel.h
    src/widget/tool/flyoutoverlaywidget.cpp
    src/widget/tool/flyoutoverlaywidget.h
    src/widget/tool/identicon.cpp
    src/widget/tool/identicon.h
    src/widget/tool/movablewidget.cpp
    src/widget/tool/movablewidget.h
    src/widget/tool/profileimporter.cpp
    src/widget/tool/profileimporter.h
    src/widget/tool/recursivesignalblocker.cpp
    src/widget/tool/recursivesignalblocker.h
    src/widget/tool/removechatdialog.cpp
    src/widget/tool/removechatdialog.h
    src/widget/tool/screengrabberchooserrectitem.cpp
    src/widget/tool/screengrabberchooserrectitem.h
    src/widget/tool/screengrabberoverlayitem.cpp
    src/widget/tool/screengrabberoverlayitem.h
    src/widget/tool/screenshotgrabber.cpp
    src/widget/tool/screenshotgrabber.h
    src/widget/tool/toolboxgraphicsitem.cpp
    src/widget/tool/toolboxgraphicsitem.h
    src/widget/translator.cpp
    src/widget/translator.h
    src/widget/widget.cpp
    src/widget/widget.h)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(${BINARY_NAME}_RESOURCES ${${BINARY_NAME}_RESOURCES}
                               platform/windows/qtox.rc)
endif()

add_definitions(-DQT_MESSAGELOGCONTEXT=1)

if(AVFOUNDATION_FOUND)
  set(${BINARY_NAME}_SOURCES
      ${${BINARY_NAME}_SOURCES} src/platform/camera/avfoundation.mm
      src/platform/camera/avfoundation.h)
endif()

if(${UPDATE_CHECK})
  add_definitions(-DUPDATE_CHECK_ENABLED=1)
  message(STATUS "Using update check")
else()
  message(STATUS "NOT using update check")
endif()

if(MINGW)
  string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
  if(CMAKE_BUILD_TYPE_LOWER MATCHES debug)
    # Allows wine to display source code file names and line numbers on crash in
    # its backtrace
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gdwarf-2")
  endif()
endif()

# the compiler flags for compiling C++ sources
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})

# Interface library to propagate code coverage flags if enabled
add_library(coverage_config INTERFACE)
option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(coverage_config INTERFACE -O0 -g --coverage)
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif()

link_libraries(coverage_config)

add_subdirectory(util)
add_subdirectory(audio)
add_subdirectory(translations)
add_subdirectory(cmake/warnings)

add_library(${BINARY_NAME}_static STATIC ${${BINARY_NAME}_FORMS}
                                         ${${BINARY_NAME}_SOURCES})
qt_add_repc_sources(${BINARY_NAME}_static src/platform/sandbox.rep)
qt_add_repc_replicas(${BINARY_NAME}_static src/platform/sandbox.rep)
target_link_libraries(${BINARY_NAME}_static ${CMAKE_REQUIRED_LIBRARIES}
                      ${ALL_LIBRARIES} coverage_config)

target_link_libraries(${BINARY_NAME}_static qtox::warnings)
target_link_libraries(${BINARY_NAME}_static util_library)
target_link_libraries(${BINARY_NAME}_static audio_library)
target_link_libraries(${BINARY_NAME}_static translations_library)

if(COMMAND qt_policy)
  # set QTP0002 policy: target properties that specify Android-specific paths
  # may contain generator expressions
  qt_policy(SET QTP0002 NEW)
endif()

qt_add_executable(${BINARY_NAME} WIN32 MACOSX_BUNDLE
                  ${${BINARY_NAME}_RESOURCES} ${SMILEY_RESOURCES} src/main.cpp)
target_link_libraries(
  ${BINARY_NAME} PRIVATE ${BINARY_NAME}_static ${CMAKE_REQUIRED_LIBRARIES}
                         ${ALL_LIBRARIES})
set_target_properties(
  ${BINARY_NAME}
  PROPERTIES
    QT_ANDROID_TARGET_SDK_VERSION 34
    QT_ANDROID_MIN_SDK_VERSION 24
    QT_ANDROID_PACKAGE_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/android/qt-${Qt6Core_VERSION}")

if(FULLY_STATIC)
  target_link_options(${BINARY_NAME} PRIVATE -static-pie)
endif()

if(EMSCRIPTEN)
  target_link_options(${BINARY_NAME} PRIVATE -sASYNCIFY -lidbfs.js)

  # Turn off testing, because there's no offscreen plugin.
  message(STATUS "Disabling testing for Emscripten")
  set(BUILD_TESTING OFF)
endif()

if(BUILD_TESTING)
  include(Testing)
endif()
include(Installation)
include(Package)

if(DEFINED ENV{IN_NIX_SHELL})
  # the qtox binary must be "wrapped" to find the Qt platform plugin and other
  # dependencies at runtime
  add_custom_command(
    TARGET ${BINARY_NAME}
    POST_BUILD
    COMMAND nix-shell --run "wrapQtApp ${BINARY_NAME}")
endif()
