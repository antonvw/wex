cmake_minimum_required(VERSION 3.6.3)

project(wex)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.65.0 COMPONENTS 
  log_setup log filesystem program_options date_time regex REQUIRED)

# to support older cmake versions that do not known 17.
# set(CMAKE_CXX_STANDARD 17)

set(user $ENV{USER} CACHE INTERNAL "keep user")

include(cmake/options.cmake)

execute_process(
  COMMAND git apply ../wxWidgets.patch
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/wxWidgets)

configure_file(
  src/lexers/LexAda.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/lexers COPYONLY)
configure_file(
  src/lexers/Catalogue.cxx 
  ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/scintilla/src COPYONLY)

add_subdirectory(external)

include(external/wxWidgets/build/cmake/toolkit.cmake)

file(READ src/core/version.cpp WEX_VERSION_CONTENTS)
string(REGEX MATCH "[ \t]+[0-9]+,[ \t\n]*[0-9]+,[ \t\n]*[0-9]+"
  WEX_VERSION ${WEX_VERSION_CONTENTS})
string(REGEX REPLACE "[ \t\n]+" "" WEX_VERSION "${WEX_VERSION}")
string(REGEX REPLACE "," "." WEX_VERSION "${WEX_VERSION}")
set(WEX_VERSION ${WEX_VERSION})
string(REGEX MATCH "[0-9]+-[0-9]+"
  WEX_COPYRIGHT ${WEX_VERSION_CONTENTS})
set(WEX_COPYRIGHT ${WEX_COPYRIGHT})

if (ENABLE_GETTEXT)
  include(FindGettext)
endif ()

find_package(ODBC)

if (ODBC_FOUND AND NOT MSVC)
  add_definitions(-DwexUSE_ODBC)
  
  include_directories(${ODBC_INCLUDE_DIR})
  include_directories(external/otl)
endif ()

enable_testing()

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/wxWidgets/build/cmake/modules")

include_directories(${Boost_INCLUDE_DIRS})

include(cmake/options-ex.cmake)
include(cmake/common.cmake)

add_subdirectory(src)

if (wexBUILD_SAMPLES)
  add_subdirectory(sample)
endif ()

if (wexBUILD_TESTS)
  add_subdirectory(test)
endif ()

add_subdirectory(locale)

set(CMAKE_INSTALL_MESSAGE LAZY)

wex_install()
wex_config()
