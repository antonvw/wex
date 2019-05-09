project(wex)

find_package(Boost 1.69.0)

if(NOT Boost_FOUND)
  message(FATAL_ERROR "boost is required")
endif()

cmake_minimum_required(VERSION 3.0.0)

set(user $ENV{USER} CACHE INTERNAL "keep user")

include(cmake/options.cmake)

execute_process(
  COMMAND git apply ../wxWidgets.patch
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/wxWidgets)

configure_file(
  src/src/lexers/LexAda.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/lexers COPYONLY)
configure_file(
  src/src/lexers/LexBash.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/lexers COPYONLY)
configure_file(
  src/src/lexers/Catalogue.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/src COPYONLY)

add_subdirectory(external)

include(external/wxWidgets/build/cmake/toolkit.cmake)

file(READ src/src/version.cpp WEX_VERSION_CONTENTS)
string(REGEX MATCH "[ \t]+[0-9]+,[0-9, ]+[0-9]+"
  WEX_VERSION ${WEX_VERSION_CONTENTS})
string(REGEX REPLACE "[ \t]+" "" WEX_VERSION ${WEX_VERSION})
string(REGEX REPLACE "," "." WEX_VERSION ${WEX_VERSION})
set(WEX_VERSION ${WEX_VERSION})
string(REGEX MATCH "[0-9]+-[0-9]+"
  WEX_COPYRIGHT ${WEX_VERSION_CONTENTS})
set(WEX_COPYRIGHT ${WEX_COPYRIGHT})

add_definitions(-DELPP_NO_DEFAULT_LOG_FILE)

if (${wexUSE_OTL} MATCHES ON)
  add_definitions(-DwexUSE_OTL)
endif ()

if (ENABLE_GETTEXT)
  include(FindGettext)
endif ()

if (wexUSE_OTL)
  include(cmake/otl.cmake)
endif ()

enable_testing()

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/wxWidgets/build/cmake/modules")

include(cmake/options-ex.cmake)
include(cmake/common.cmake)

add_subdirectory(src)
