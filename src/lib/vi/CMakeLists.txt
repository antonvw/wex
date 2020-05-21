file(GLOB SRCS "*.cpp")

add_library(
  wex-vi
  ${SRCS})

set_target_properties(wex-vi PROPERTIES DEBUG_POSTFIX "d")
