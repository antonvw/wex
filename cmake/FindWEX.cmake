# Tries to find wex include dir and libraries
#
# Usage of this module as follows:
#
#     find_package(WEX)
#
# Variables defined by this module:
#
#  wex_FOUND          System has wex libraries, include and library dirs found.
#  wex_INCLUDE_DIR    The wex include directory.
#  wex_LIB_DIR        The wex lib directory.
#  wex_LIBRARIES      The wex libraries.

if (wexBUILD_SHARED)
  add_definitions(-DBOOST_LOG_DYN_LINK)

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -undefined dynamic_lookup")
  endif()
else ()
  set(Boost_USE_STATIC_LIBS ON)
endif ()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.75.0 COMPONENTS
  log_setup log filesystem program_options date_time regex json REQUIRED)

find_package(ODBC QUIET)

if (ODBC_FOUND)
  add_definitions(-DwexUSE_ODBC)
else ()
  set(ODBC_LIBRARIES "")
endif ()

if (WIN32)
  add_definitions(-D__WXMSW__)

  set(PLATFORM "msw")
elseif (APPLE AND IPHONE)
  add_definitions(-D__WXOSX_IPHONE__)

  set(PLATFORM "osx_iphone")
elseif (APPLE)
  find_package(ICONV)
  find_package(ZLIB)
  add_definitions(-D__WXOSX_COCOA__)

  set(PLATFORM "osx_cocoa")

  set(cpp_LIBRARIES stdc++)

  set(apple_LIBRARIES
    wxjpeg-3.3
    wxpng-3.3
    ${ICONV_LIBRARIES}
    ${ZLIB_LIBRARIES})
elseif (UNIX)
  add_definitions(-D__WXGTK3__ -D__WXGTK__)

  set(PLATFORM "gtk3")

  if (CENTOS)
    set (cpp_std_LIBRARIES
      /usr/gnat/lib64/libstdc++.a
      /usr/gnat/lib64/libstdc++fs.a)
  else ()
    set (cpp_std_LIBRARIES
      stdc++
      stdc++fs)
  endif ()

  find_package(JPEG)
  find_package(PNG)
  find_package(X11)
  find_package(ZLIB)

  set(cpp_LIBRARIES
    ${cpp_std_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${X11_LIBRARIES}
    ${ZLIB_LIBRARIES}
    -lpthread
    -ldl
    -lc
    -lm
    -lgtk-3
    -lgdk-3
    -latk-1.0
    -lgio-2.0
    -lpangocairo-1.0
    -lgdk_pixbuf-2.0
    -lcairo-gobject
    -lpango-1.0
    -lcairo
    -lgobject-2.0
    -lglib-2.0)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

if (APPLE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -framework AudioToolbox \
    -framework WebKit \
    -framework CoreFoundation \
    -framework Security \
    -framework Cocoa \
    -framework IOKit")
endif()

if (MSVC)
  if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    set(USE_DEBUG "d")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS \
    /Zc:__cplusplus")

  set(wx_LIBRARIES
    wx${PLATFORM}33u${USE_DEBUG}_aui
    wx${PLATFORM}33u${USE_DEBUG}_stc
    wx${PLATFORM}33u${USE_DEBUG}_html
    wx${PLATFORM}33u${USE_DEBUG}_core
    wx${PLATFORM}33u${USE_DEBUG}_qa
    wx${PLATFORM}33u${USE_DEBUG}_gl
    wxbase33u${USE_DEBUG}
    wxbase33u${USE_DEBUG}_net
    wxjpeg
    wxpng
    wxzlib
    wxscintilla${USE_DEBUG}
    comctl32.lib
    Rpcrt4.lib)
else()
  set(wx_LIBRARIES
    wx_${PLATFORM}u_aui-3.3
    wx_${PLATFORM}u_stc-3.3
    wx_${PLATFORM}u_html-3.3
    wx_${PLATFORM}u_core-3.3
    wx_baseu-3.3
    wx_baseu_net-3.3)

  if (NOT APPLE AND NOT wexBUILD_SHARED)
    set(wx_LIBRARIES ${wx_LIBRARIES} wxscintilla-3.3)
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

include_directories(${Boost_INCLUDE_DIRS})

# these should be the same as in common.cmake
set(wex_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/wex/23.04")
set(wex_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib")

set(wex_LIBRARIES
  wex-del${USE_DEBUG}
  wex-vcs${USE_DEBUG}
  wex-stc${USE_DEBUG}
  wex-vi${USE_DEBUG}
  wex-ex${USE_DEBUG}
  wex-ctags${USE_DEBUG}
  wex-ui${USE_DEBUG}
  wex-common${USE_DEBUG}
  wex-data${USE_DEBUG}
  wex-syntax${USE_DEBUG}
  wex-factory${USE_DEBUG}
  wex-core${USE_DEBUG}
  ${wx_LIBRARIES}
  ${apple_LIBRARIES}
  ${Boost_LIBRARIES}
  ${cpp_LIBRARIES}
  ${ODBC_LIBRARIES})

set(wex_FOUND ON)
