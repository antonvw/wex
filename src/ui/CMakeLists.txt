file(GLOB SRCS "*.cpp")

add_library(
  wex-ui
  ${SRCS})

add_subdirectory(ctags)
