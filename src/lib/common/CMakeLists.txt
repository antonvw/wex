file(GLOB SRCS "*.cpp")

add_library(
  wex-common
  ${SRCS})

set_target_properties(wex-common PROPERTIES DEBUG_POSTFIX "d")
