include(
  ${wxWidgets_USE_FILE})

set(SRCS 
 dir.cpp
 dirctrl.cpp
 frame.cpp
 listview.cpp
 listviewfile.cpp
 stc.cpp
 textfile.cpp
 util.cpp)
  
include_directories(
  ../../include/)

add_library(
  wxex-rep
  ${SRCS})
