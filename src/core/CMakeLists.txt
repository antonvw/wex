file(GLOB SRCS 
  "*.cpp"
  "../../external/pugixml/src/*.cpp")

add_library(
  wex-core
  ${SRCS})
