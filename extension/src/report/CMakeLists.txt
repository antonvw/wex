file(GLOB SRCS "*.cpp")
  
add_library(
  wex-rep
  ${SRCS})

set_target_properties(wex-rep PROPERTIES DEBUG_POSTFIX "d")
