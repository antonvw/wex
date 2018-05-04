project(wxExtension)

cmake_minimum_required(VERSION 3.0.0)

set(user $ENV{USER} CACHE INTERNAL "keep user")

include(cmake/options.cmake)

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

if (${wxExUSE_OTL} MATCHES ON)
  add_definitions(-DwxExUSE_OTL)
endif ()

if (ENABLE_GETTEXT)
  include(FindGettext)
endif ()

if (wxExUSE_OTL)
  include(cmake/otl.cmake)
endif ()

enable_testing()

include(cmake/options-ex.cmake)
include(cmake/common.cmake)

add_subdirectory(extension)

if (wxExUSE_SP)
  add_subdirectory(syncped)
endif ()

if (wxExUSE_SOQ)
  add_subdirectory(syncodbcquery)
endif ()

if (wxExBUILD_SSS)
  add_subdirectory(syncsocketserver)
endif ()
