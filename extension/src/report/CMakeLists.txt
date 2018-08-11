file(GLOB SRCS "*.cpp")
  
add_library(
  wxex-rep
  ${SRCS})

set_target_properties(wxex-rep PROPERTIES DEBUG_POSTFIX "d")
