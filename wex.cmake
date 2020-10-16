set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.65.0 COMPONENTS 
  filesystem program_options date_time regex REQUIRED)

if (NOT Boost_FOUND)
  message(FATAL_ERROR "boost is required")
endif()

link_directories("/usr/local/lib/wex")

if (WIN32)
  add_definitions(-D__WXMSW__)
  set(PLATFORM "msw")
elseif (APPLE AND IPHONE)
  add_definitions(-D__WXOSX_IPHONE__)
  set(PLATFORM "osx_iphone")
elseif (APPLE)
  add_definitions(-D__WXOSX_COCOA__)
  set(PLATFORM "osx_cocoa")
  set(cpp_LIBRARIES stdc++)
  set(extra_LIBRARIES wx_${PLATFORM}u_media-3.1 wxjpeg-3.1 wxpng-3.1)
elseif (UNIX)
  add_definitions(-D__WXGTK3__ -D__WXGTK__)
  set(PLATFORM "gtk3")
  set(cpp_LIBRARIES /usr/gnat/lib64/libstdc++.a /usr/gnat/lib64/libstdc++fs.a m 
    pthread dl /usr/lib64/libjpeg.so /usr/lib64/libpng.so /usr/lib64/libz.so 
    -lc -lpthread -ldl /usr/lib64/libSM.so /usr/lib64/libICE.so 
    /usr/lib64/libX11.so /usr/lib64/libXext.so -lgtk-3 -lgdk-3 -latk-1.0 
    -lgio-2.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo-gobject -lpango-1.0 
    -lcairo -lgobject-2.0 -lglib-2.0 /usr/lib64/libXtst.so)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

include_directories("/usr/local/include/wex")
  
if (APPLE)
  set(CMAKE_EXE_LINKER_FLAGS "-framework AudioToolbox -framework WebKit /usr/lib/libz.dylib /usr/lib/libiconv.dylib -framework CoreFoundation -framework Security -framework Carbon -framework Cocoa -framework IOKit")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g")

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
  wx_${PLATFORM}u_aui-3.1
  wx_${PLATFORM}u_adv-3.1
  wx_${PLATFORM}u_stc-3.1
  wx_${PLATFORM}u_html-3.1
  wx_${PLATFORM}u_core-3.1
  wx_baseu-3.1 
  wx_baseu_net-3.1 
  wxscintilla-3.1
  ${extra_LIBRARIES}
  ${Boost_LIBRARIES}
  ${cpp_LIBRARIES})
