project(wex)

cmake_minimum_required(VERSION 3.0.0)

set(user $ENV{USER} CACHE INTERNAL "keep user")

include(cmake/options.cmake)

configure_file(
  extension/src/lexers/LexAda.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/lexers COPYONLY)
configure_file(
  extension/src/lexers/LexBash.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/lexers COPYONLY)
configure_file(
  extension/src/lexers/Catalogue.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/src COPYONLY)

add_subdirectory(external)

include(external/wxWidgets/build/cmake/toolkit.cmake)

file(READ extension/src/version.cpp EX_VERSION_CONTENTS)
string(REGEX MATCH "[ \t]+[0-9]+,[0-9, ]+[0-9]+"
  EX_VERSION ${EX_VERSION_CONTENTS})
string(REGEX REPLACE "[ \t]+" "" EX_VERSION ${EX_VERSION})
string(REGEX REPLACE "," "." EX_VERSION ${EX_VERSION})
set(EX_VERSION ${EX_VERSION})
string(REGEX MATCH "[0-9]+-[0-9]+"
  EX_COPYRIGHT ${EX_VERSION_CONTENTS})
set(EX_COPYRIGHT ${EX_COPYRIGHT})

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

add_subdirectory(extension)

if (wexBUILD_SP)
  add_subdirectory(syncped)
endif ()

if (wexBUILD_SOQ)
  add_subdirectory(syncodbcquery)
endif ()

if (wexBUILD_SSS)
  add_subdirectory(syncsocketserver)
endif ()
