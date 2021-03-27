file(GLOB SRCS 
  "../../external/ctags/libreadtags/*.c"
  "*.cpp")

add_library(
  wex-vi
  ${SRCS})
