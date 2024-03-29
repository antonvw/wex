if (wexBUILD_TIDY)
  file(GLOB SRCS "*.cpp")
else ()
  file(GLOB SRCS 
  "*.cpp"
  "../../external/pugixml/src/*.cpp"
  "../../external/wxMaterialDesignArtProvider/MaterialDesign/*.cpp")
endif ()

add_library(
  wex-core
  ${SRCS})
