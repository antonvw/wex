file(GLOB SRCS "*.cpp")
  
add_library(
  wxex-rep
  ${SRCS})

if (NOT (CMAKE_BUILD_TYPE MATCHES "Coverage"))
  cotire(wxex-rep)
endif ()
  
set_target_properties(wxex-rep PROPERTIES DEBUG_POSTFIX "d")
