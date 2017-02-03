FILE(GLOB SRCS "*.c" "*.cpp" "../pugixml/src/*.cpp" "shunting-yard/*.cpp")
  
add_library(
  wxex 
  ${SRCS})
  
set_target_properties(wxex PROPERTIES DEBUG_POSTFIX "d")
