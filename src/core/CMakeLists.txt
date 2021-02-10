file(GLOB SRCS 
  "*.cpp"
  "../../external/ctags/libreadtags/*.c"
  "../../external/pugixml/src/*.cpp")

add_library(
  wex-core
  ${SRCS})
