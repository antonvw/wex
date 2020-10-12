project(wex)

cmake_minimum_required(VERSION 3.6.3)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.65.0 COMPONENTS 
  filesystem program_options date_time regex REQUIRED)

# to support older cmake versions that do not known 17.
# set(CMAKE_CXX_STANDARD 17)

set(user $ENV{USER} CACHE INTERNAL "keep user")

include(cmake/options.cmake)

execute_process(
  COMMAND git apply ../wxWidgets.patch
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/wxWidgets)

configure_file(
  src/lib/lexers/LexAda.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/lexers COPYONLY)
configure_file(
  src/lib/lexers/Catalogue.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/src COPYONLY)

add_subdirectory(external)

include(external/wxWidgets/build/cmake/toolkit.cmake)

file(READ src/lib/core/version.cpp WEX_VERSION_CONTENTS)
string(REGEX MATCH "[ \t]+[0-9]+,[ \t\n]*[0-9]+,[ \t\n]*[0-9]+"
  WEX_VERSION ${WEX_VERSION_CONTENTS})
string(REGEX REPLACE "[ \t\n]+" "" WEX_VERSION "${WEX_VERSION}")
string(REGEX REPLACE "," "." WEX_VERSION "${WEX_VERSION}")
set(WEX_VERSION ${WEX_VERSION})
string(REGEX MATCH "[0-9]+-[0-9]+"
  WEX_COPYRIGHT ${WEX_VERSION_CONTENTS})
set(WEX_COPYRIGHT ${WEX_COPYRIGHT})

add_definitions(-DELPP_NO_DEFAULT_LOG_FILE)

if (ENABLE_GETTEXT)
  include(FindGettext)
endif ()

find_package(odbc)

if (ODBC_FOUND AND NOT MSVC)
  add_definitions(-DwexUSE_OTL)
  
  include_directories(${ODBC_INCLUDE_DIR})
  include_directories(external/otl)
endif ()

enable_testing()

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/wxWidgets/build/cmake/modules")

include(cmake/options-ex.cmake)
include(cmake/common.cmake)

add_subdirectory(src)

wex_install()
wex_config()
