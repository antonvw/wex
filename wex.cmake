set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.65.0 COMPONENTS 
  filesystem program_options date_time regex REQUIRED)

if(NOT Boost_FOUND)
  message(FATAL_ERROR "boost is required")
endif()

link_directories("/usr/local/Cellar/wex/lib")
include_directories("/usr/local/Cellar/wex/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")
set(CMAKE_EXE_LINKER_FLAGS "-framework AudioToolbox -framework WebKit /usr/lib/libz.dylib /usr/lib/libiconv.dylib -framework CoreFoundation -framework Security -framework Carbon -framework Cocoa -framework IOKit")
set(wex_LIBRARIES
  wex-reportd 
  wex-commond 
  wex-stcd 
  wex-uid 
  wex-vid
  wex-commond 
  wex-stcd 
  wex-uid 
  wex-datad 
  wex-cored 
  wx_osx_cocoau_aui-3.1
  wx_osx_cocoau_adv-3.1
  wx_osx_cocoau_stc-3.1
  wx_osx_cocoau_html-3.1
  wx_osx_cocoau_core-3.1
  wx_osx_cocoau_media-3.1
  wx_baseu-3.1 
  wx_baseu_net-3.1 
  wxscintilla-3.1
  wxjpeg-3.1
  wxpng-3.1
  ${Boost_LIBRARIES}
  stdc++)
