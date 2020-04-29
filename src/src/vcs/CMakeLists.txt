file(GLOB SRCS "*.cpp")

add_library(
  wex-vcs
  ${SRCS})

set_target_properties(wex-vcs PROPERTIES DEBUG_POSTFIX "d")
