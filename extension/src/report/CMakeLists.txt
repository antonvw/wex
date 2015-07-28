FILE(GLOB SRCS "*.cpp")
  
include_directories(
  ../../include/)

add_library(
  wxex-rep
  ${SRCS})
  
set_target_properties(wxex-rep PROPERTIES DEBUG_POSTFIX "d")
