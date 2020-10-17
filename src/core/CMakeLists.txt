file(GLOB SRCS 
  "*.cpp"
  "../../external/ctags/libreadtags/*.c"
  "../../external/easyloggingpp/src/*.cc"
  "../../external/pugixml/src/*.cpp")

add_library(
  wex-core
  ${SRCS})
