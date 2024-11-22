# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2019 by The qTox Project Contributors
# Copyright © 2024 The TokTok team.

################################################################################
#
# :: Dependencies
#
################################################################################

# This should go into subdirectories later.
find_package(PkgConfig        REQUIRED)
find_package(Qt6Concurrent    REQUIRED)
find_package(Qt6Core          REQUIRED)
find_package(Qt6Gui           REQUIRED)
find_package(Qt6Linguist      REQUIRED)
find_package(Qt6Network       REQUIRED)
find_package(Qt6OpenGL        REQUIRED)
find_package(Qt6Svg           REQUIRED)
find_package(Qt6Test          REQUIRED)
find_package(Qt6Widgets       REQUIRED)
find_package(Qt6Xml           REQUIRED)

function(add_dependency)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} ${ARGN} PARENT_SCOPE)
endfunction()

# Everything links to these Qt libraries.
add_dependency(
  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::OpenGL
  Qt6::Svg
  Qt6::Widgets
  Qt6::Xml)

include(CMakeParseArguments)

function(search_dependency pkg)
  set(options OPTIONAL STATIC_PACKAGE)
  set(oneValueArgs PACKAGE LIBRARY FRAMEWORK HEADER)
  set(multiValueArgs)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Try pkg-config first.
  if(NOT ${pkg}_FOUND AND arg_PACKAGE)
    pkg_check_modules(${pkg} ${arg_PACKAGE})
  endif()

  # Then, try OSX frameworks.
  if(NOT ${pkg}_FOUND AND arg_FRAMEWORK)
    find_library(${pkg}_LIBRARIES
            NAMES ${arg_FRAMEWORK}
            PATHS ${CMAKE_OSX_SYSROOT}/System/Library
            PATH_SUFFIXES Frameworks
            NO_DEFAULT_PATH
    )
    if(${pkg}_LIBRARIES)
      set(${pkg}_FOUND TRUE)
    endif()
  endif()

  # Last, search for the library itself globally.
  if(NOT ${pkg}_FOUND AND arg_LIBRARY)
    find_library(${pkg}_LIBRARIES NAMES ${arg_LIBRARY})
    if(arg_HEADER)
        find_path(${pkg}_INCLUDE_DIRS NAMES ${arg_HEADER})
    endif()
    if(${pkg}_LIBRARIES AND (${pkg}_INCLUDE_DIRS OR NOT arg_HEADER))
      set(${pkg}_FOUND TRUE)
    endif()
  endif()

  if(NOT ${pkg}_FOUND)
    if(NOT arg_OPTIONAL)
      message(FATAL_ERROR "${pkg} package, library or framework not found")
    else()
      message(STATUS "${pkg} not found")
    endif()
  else()
    if(arg_STATIC_PACKAGE)
      set(maybe_static _STATIC)
    else()
      set(maybe_static "")
    endif()

    message(STATUS ${pkg} " LIBRARY_DIRS: " "${${pkg}${maybe_static}_LIBRARY_DIRS}" )
    message(STATUS ${pkg} " INCLUDE_DIRS: " "${${pkg}${maybe_static}_INCLUDE_DIRS}" )
    message(STATUS ${pkg} " CFLAGS_OTHER: " "${${pkg}${maybe_static}_CFLAGS_OTHER}" )
    message(STATUS ${pkg} " LIBRARIES:    " "${${pkg}${maybe_static}_LIBRARIES}" )

    link_directories(${${pkg}${maybe_static}_LIBRARY_DIRS})
    include_directories(${${pkg}${maybe_static}_INCLUDE_DIRS})

    foreach(flag ${${pkg}${maybe_static}_CFLAGS_OTHER})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
    endforeach()

    set(ALL_LIBRARIES ${ALL_LIBRARIES} ${${pkg}${maybe_static}_LIBRARIES} PARENT_SCOPE)
    message(STATUS "${pkg} found")
  endif()

  set(${pkg}_FOUND ${${pkg}_FOUND} PARENT_SCOPE)
endfunction()

search_dependency(LIBAVCODEC          PACKAGE libavcodec)
search_dependency(LIBAVDEVICE         PACKAGE libavdevice)
search_dependency(LIBAVFORMAT         PACKAGE libavformat)
search_dependency(LIBAVUTIL           PACKAGE libavutil)
search_dependency(LIBEXIF             PACKAGE libexif)
search_dependency(LIBQRENCODE         PACKAGE libqrencode)
search_dependency(LIBSODIUM           PACKAGE libsodium)
search_dependency(LIBSWSCALE          PACKAGE libswscale)
search_dependency(SQLCIPHER           PACKAGE sqlcipher)
search_dependency(VPX                 PACKAGE vpx)

if(DESKTOP_NOTIFICATIONS)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    search_dependency(LIBNOTIFY       PACKAGE libnotify)
  else()
    message(FATAL_ERROR "Desktop notifications are not supported on this platform")
  endif()
endif()

if(SPELL_CHECK)
    find_package(KF6Sonnet)
    if(KF6Sonnet_FOUND)
      add_definitions(-DSPELL_CHECKING)
      add_dependency(KF6::SonnetUi)
    else()
      message(WARNING "Sonnet not found. Spell checking will be disabled.")
    endif()
endif()

# Try to find cmake toxcore libraries
if(WIN32 OR ANDROID)
  search_dependency(TOXCORE             PACKAGE toxcore          OPTIONAL STATIC_PACKAGE)
else()
  search_dependency(TOXCORE             PACKAGE toxcore          OPTIONAL)
endif()

# If not found, use automake toxcore libraries
# We only check for TOXCORE, because the other two are gone in 0.2.0.
if (NOT TOXCORE_FOUND)
  search_dependency(TOXCORE         PACKAGE libtoxcore)
  search_dependency(TOXAV           PACKAGE libtoxav)
endif()

search_dependency(OPENAL            PACKAGE openal)

if (ANDROID)
  find_library(OPENSL_LIBRARY NAMES OpenSLES)

  if(NOT OPENSL_LIBRARY)
    message(FATAL_ERROR "OpenSLES library not found")
  endif()

  add_dependency(${OPENSL_LIBRARY})
endif()

if (PLATFORM_EXTENSIONS AND UNIX AND NOT APPLE AND NOT ANDROID)
  # Automatic auto-away support. (X11 also using for capslock detection)
  search_dependency(X11               PACKAGE x11 OPTIONAL)
  search_dependency(XSS               PACKAGE xscrnsaver OPTIONAL)
endif()

if(APPLE)
  search_dependency(AVFOUNDATION      FRAMEWORK AVFoundation)
  search_dependency(COREMEDIA         FRAMEWORK CoreMedia   )
  search_dependency(COREGRAPHICS      FRAMEWORK CoreGraphics)

  search_dependency(FOUNDATION        FRAMEWORK Foundation  OPTIONAL)
  search_dependency(IOKIT             FRAMEWORK IOKit       OPTIONAL)
endif()

if(WIN32)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} strmiids)
  # Qt doesn't provide openssl on windows
  search_dependency(OPENSSL           PACKAGE openssl)
endif()

if (NOT GIT_DESCRIBE)
  execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT GIT_DESCRIBE)
    set(GIT_DESCRIBE "Nightly")
  endif()
endif()

add_definitions(
  -DGIT_DESCRIBE="${GIT_DESCRIBE}"
)

if (NOT GIT_VERSION)
  execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT GIT_VERSION)
    set(GIT_VERSION "build without git")
  endif()
endif()

add_definitions(
  -DGIT_VERSION="${GIT_VERSION}"
)

if (NOT GIT_DESCRIBE_EXACT)
  execute_process(
    COMMAND git describe --exact-match --tags HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_EXACT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT GIT_DESCRIBE_EXACT)
    set(GIT_DESCRIBE_EXACT "Nightly")
  endif()
endif()

add_definitions(
  -DGIT_DESCRIBE_EXACT="${GIT_DESCRIBE_EXACT}"
)

set(APPLE_EXT False)
if (FOUNDATION_FOUND AND IOKIT_FOUND)
  set(APPLE_EXT True)
endif()

set(X11_EXT False)
if (X11_FOUND AND XSS_FOUND)
  set(X11_EXT True)
endif()

if (PLATFORM_EXTENSIONS)
  if (${APPLE_EXT} OR ${X11_EXT} OR WIN32)
    add_definitions(
      -DQTOX_PLATFORM_EXT
    )
    message(STATUS "Using platform extensions")
  else()
    message(WARNING "Not using platform extensions, dependencies not found")
    set(PLATFORM_EXTENSIONS OFF)
  endif()
endif()

add_definitions(
  -DLOG_TO_FILE=1
)

add_definitions(-DCMAKE_BUILD)
