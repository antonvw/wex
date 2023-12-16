file(GLOB SRCS 
  "*.cpp"
  "../../external/pugixml/src/*.cpp"
  "../../external/wxMaterialDesignArtProvider/MaterialDesign/*.cpp")

add_library(
  wex-core
  ${SRCS})
