file(GLOB SRCS "*.cpp")
  
add_library(
  wex-report
  ${SRCS})

set_target_properties(wex-report PROPERTIES DEBUG_POSTFIX "d")
