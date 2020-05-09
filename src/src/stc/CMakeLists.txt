file(GLOB SRCS "*.cpp")

add_library(
  wex-stc
  ${SRCS})

set_target_properties(wex-stc PROPERTIES DEBUG_POSTFIX "d")
