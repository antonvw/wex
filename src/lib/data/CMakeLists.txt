file(GLOB SRCS "*.cpp")

add_library(
  wex-data
  ${SRCS})

set_target_properties(wex-data PROPERTIES DEBUG_POSTFIX "d")
