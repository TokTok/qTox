# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2019 by The qTox Project Contributors
# Copyright © 2024-2025 The TokTok team.

################################################################################
#
# :: Dependencies
#
################################################################################

# This should go into subdirectories later.
find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS
  Concurrent
  Core
  Gui
  Linguist
  Network
  RemoteObjects
  Svg
  Test
  Widgets
  Xml)

message(STATUS "Qt6 found at ${QT6_INSTALL_PREFIX}")

function(add_dependency)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} ${ARGN} PARENT_SCOPE)
endfunction()

# Everything links to these Qt libraries.
add_dependency(
  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::RemoteObjects
  Qt6::Svg
  Qt6::Widgets
  Qt6::Xml)

if(QT_FEATURE_dbus)
  # Optional dependency for desktop notifications.
  find_package(Qt6DBus)

  if(Qt6DBus_FOUND)
    if(Qt6DBus_DIR MATCHES "^${Qt6_DIR}")
      add_dependency(Qt6::DBus)
      message(STATUS "Using DBus for desktop notifications")
    else()
      message(STATUS "Not using DBus (Qt6DBus found in ${Qt6DBus_DIR}, which is outside Qt6 installation ${Qt6_DIR})")
    endif()
  endif()
endif()

include(CMakeParseArguments)

set(TOXCORE_MINIMUM_VERSION "0.2.20")

function(search_dependency pkg)
  set(options OPTIONAL STATIC_PACKAGE)
  set(oneValueArgs PACKAGE LIBRARY FRAMEWORK HEADER MINIMUM_VERSION)
  set(multiValueArgs)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Try pkg-config first.
  if(NOT ${pkg}_FOUND AND arg_PACKAGE)
    pkg_check_modules(${pkg} ${arg_PACKAGE})
    if(${pkg} AND ${pkg} MATCHES "TOXCORE" AND "${TOXCORE_VERSION}" VERSION_LESS "${arg_MINIMUM_VERSION}")
      message(FATAL_ERROR "Minimum ${arg_PACKAGE} version is: ${arg_MINIMUM_VERSION}")
    endif()
  endif()

  # Then, try macOS frameworks.
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
search_dependency(LIBSWSCALE          PACKAGE libswscale)
search_dependency(SQLCIPHER           PACKAGE sqlcipher)

if(EMSCRIPTEN)
  search_dependency(TOMCRYPT          LIBRARY tomcrypt)
endif()

if(APPLE)
  search_dependency(LIBCRYPTO         PACKAGE libcrypto)
endif()

if(SPELL_CHECK)
  find_package(KF6Sonnet)
  if(KF6Sonnet_FOUND)
    message(STATUS "KF6Sonnet_DIR: " "${KF6Sonnet_DIR}")
    add_definitions(-DSPELL_CHECKING)
    add_dependency(KF6::SonnetUi)
  else()
    message(WARNING "Sonnet not found. Spell checking will be disabled.")
  endif()
endif()

# Try to find cmake toxcore libraries
if(WIN32 OR ANDROID OR FULLY_STATIC)
  search_dependency(TOXCORE             PACKAGE toxcore             LIBRARY toxcore          OPTIONAL STATIC_PACKAGE MINIMUM_VERSION ${TOXCORE_MINIMUM_VERSION})
else()
  search_dependency(TOXCORE             PACKAGE toxcore             LIBRARY toxcore          OPTIONAL MINIMUM_VERSION ${TOXCORE_MINIMUM_VERSION})
  search_dependency(TOXAV               PACKAGE toxav               LIBRARY toxav            OPTIONAL)
  search_dependency(TOXENCRYPTSAVE      PACKAGE toxencryptsave      LIBRARY toxencryptsave   OPTIONAL)
endif()

# If not found, use automake toxcore libraries
# We only check for TOXCORE, because the other two are gone in 0.2.0.
if (NOT TOXCORE_FOUND)
  search_dependency(TOXCORE         PACKAGE libtoxcore MINIMUM_VERSION ${TOXCORE_MINIMUM_VERSION})
  search_dependency(TOXAV           PACKAGE libtoxav)
endif()

search_dependency(LIBSODIUM         PACKAGE libsodium)
search_dependency(OPUS              PACKAGE opus)
search_dependency(VPX               PACKAGE vpx)

search_dependency(OPENAL            PACKAGE openal)

search_dependency(V4L2              PACKAGE v4l2 LIBRARY v4l2 OPTIONAL)

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
  add_dependency(strmiids)
  # Qt doesn't provide openssl on windows
  search_dependency(OPENSSL           PACKAGE openssl)
endif()

if(NOT WIN32)
  search_dependency(LIBUNWIND         PACKAGE libunwind OPTIONAL)
  if(LIBUNWIND_FOUND)
    message(STATUS "Using libunwind for stack traces")
    set_property(SOURCE src/platform/stacktrace.cpp APPEND PROPERTY COMPILE_DEFINITIONS QTOX_USE_LIBUNWIND)
  endif()
endif()

if(QT_FEATURE_static)
  if(NOT EMSCRIPTEN)
    add_dependency(Qt6::QOffscreenIntegrationPlugin)
  endif()
  if(LINUX)
    add_dependency(
      Qt6::QLinuxFbIntegrationPlugin
      Qt6::QVncIntegrationPlugin
      Qt6::QXcbIntegrationPlugin
      Qt6::QWaylandIntegrationPlugin)
  endif()
  set(KIMG_FORMATS qoi)
  foreach(fmt ${KIMG_FORMATS})
    # fmt toupper
    string(TOUPPER ${fmt} fmt_lib)
    find_library(KIMG_${fmt_lib}_LIBRARY kimg_${fmt}
                PATHS "${QT6_INSTALL_PREFIX}/plugins/imageformats")
    if(KIMG_${fmt_lib}_LIBRARY)
      message(STATUS "Found ${fmt_lib} imageformats plugin: ${KIMG_${fmt_lib}_LIBRARY}")
      add_dependency(${KIMG_${fmt_lib}_LIBRARY})
      set_property(SOURCE src/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS QTOX_USE_KIMG_${fmt_lib})
    else()
      message(STATUS "Did not find ${fmt_lib} imageformats plugin")
    endif()
  endforeach()
endif()

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

if (NOT GIT_DESCRIBE)
  execute_process(
    COMMAND git describe --tags --match v*
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT GIT_DESCRIBE)
    set(GIT_DESCRIBE "Nightly")
  endif()
endif()

if (NOT GIT_DESCRIBE_EXACT)
  execute_process(
    COMMAND git describe --tags --match v* --exact-match
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_EXACT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT GIT_DESCRIBE_EXACT)
    set(GIT_DESCRIBE_EXACT "Nightly")
  endif()
endif()

message(STATUS "Git version: ${GIT_VERSION}")
message(STATUS "Git describe: ${GIT_DESCRIBE}")
message(STATUS "Git describe exact: ${GIT_DESCRIBE_EXACT}")

# Generate version.cpp with the above version information.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp
  @ONLY
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
    add_definitions(-DQTOX_PLATFORM_EXT)
    message(STATUS "Using platform extensions")
  else()
    message(WARNING "Not using platform extensions, dependencies not found")
    set(PLATFORM_EXTENSIONS OFF)
  endif()
endif()

add_definitions(-DLOG_TO_FILE=1)
