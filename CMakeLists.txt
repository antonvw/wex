cmake_minimum_required(VERSION 3.21)

project(wex)

if (wexBUILD_SHARED)
  set(BUILD_SHARED_LIBS YES)
  set(wxBUILD_SHARED ON)
  add_definitions(-DBOOST_LOG_DYN_LINK)
  
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS 
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
  endif()
else ()
  set(Boost_USE_STATIC_LIBS ON)
  set(wxBUILD_SHARED OFF)
endif ()
  
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.81.0 COMPONENTS 
  log_setup log filesystem program_options date_time regex json url REQUIRED)
  
set(WEX_CXX_STANDARD 23) # see also FindWEX.cmake, c_cpp_properties.json
set(CMAKE_CXX_STANDARD ${WEX_CXX_STANDARD})
set(user $ENV{USER} CACHE INTERNAL "keep user")

include(cmake/options-wx.cmake)

execute_process(
  COMMAND git apply ${CMAKE_SOURCE_DIR}/external/wxWidgets.patch
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/wxWidgets)

execute_process(
  COMMAND git apply ${CMAKE_SOURCE_DIR}/external/lexilla.patch
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/wxWidgets/src/stc/lexilla)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(external/doctest/doctest)

add_subdirectory(external)

include(external/wxWidgets/build/cmake/toolkit.cmake)

file(READ src/core/version.cpp WEX_VERSION_CONTENTS)
string(REGEX MATCH "[ \t]+([0-9]+),[ \t\n]*([0-9]+),[ \t\n]*([0-9]+)"
  WEX_VERSION ${WEX_VERSION_CONTENTS})
set(WEX_VERSION ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3})
set(WEX_VERSION_INCLUDE ${CMAKE_MATCH_1}.${CMAKE_MATCH_2})
string(REGEX MATCH "\".c.[ 0-9a-zA-Z,.\-]+\""
  WEX_COPYRIGHT ${WEX_VERSION_CONTENTS})
set(WEX_COPYRIGHT ${WEX_COPYRIGHT})

if (wexENABLE_GETTEXT)
  include(FindGettext)
endif ()

if (wexENABLE_ODBC)
  find_package(ODBC)

  if (ODBC_FOUND AND NOT MSVC)
    add_definitions(-DwexUSE_ODBC)
    
    include_directories(${ODBC_INCLUDE_DIR})
    include_directories(external/otl)
  endif ()
endif ()

enable_testing()

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/wxWidgets/build/cmake/modules")

include(cmake/options.cmake)
include(cmake/common.cmake)

if (wexENABLE_ASAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif ()

# See also FindWEX
set(wex_own_LIBRARIES
  wex-del wex-vcs wex-stc wex-vi wex-ex wex-ctags
  wex-ui wex-common wex-data wex-syntax wex-factory wex-test wex-core)

separate_arguments(wex_own_LIBRARIES)

add_subdirectory(ci)
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
