file(GLOB SRCS 
  "*.cpp"
  "../../external/ctags/*.c"
  "../../external/easyloggingpp/src/*.cc"
  "../../external/pugixml/src/*.cpp"
  "../../external/shunting-yard/*.cpp")

add_library(
  wxex 
  ${SRCS})

if (NOT (CMAKE_BUILD_TYPE MATCHES "Coverage"))
  cotire(wxex)
endif()
  
set_target_properties(wxex PROPERTIES DEBUG_POSTFIX "d")
