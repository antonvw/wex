file(GLOB SRCS 
  "*.cpp")

add_library(
  wex-ex
  ${SRCS})

add_subdirectory(vi)
