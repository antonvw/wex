file(GLOB SRCS "*.cpp")

add_library(
  wex-lexer
  ${SRCS})

set_target_properties(wex-lexer PROPERTIES DEBUG_POSTFIX "d")
