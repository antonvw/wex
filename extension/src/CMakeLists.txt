include_directories(
  ../include/)

FILE(GLOB SRCS "*.cpp" "shunting-yard/*.cpp")
  
add_library(
  wxex 
  ${SRCS})
  
set_target_properties(wxex PROPERTIES DEBUG_POSTFIX "d")
