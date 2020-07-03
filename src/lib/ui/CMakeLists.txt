file(GLOB SRCS "*.cpp")

add_library(
  wex-ui
  ${SRCS})

set_target_properties(wex-ui PROPERTIES DEBUG_POSTFIX "d")
