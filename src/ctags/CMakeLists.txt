file(GLOB SRCS 
  "../../external/ctags/libreadtags/*.c"
  "*.cpp")

add_library(
  wex-ctags
  ${SRCS})
