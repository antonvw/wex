file(GLOB SRCS 
  "*.cpp"
  "../../external/ctags/read/*.c"
  "../../external/easyloggingpp/src/*.cc"
  "../../external/pugixml/src/*.cpp")

add_library(
  wex 
  ${SRCS})

set_target_properties(wex PROPERTIES DEBUG_POSTFIX "d")
