if (wexBUILD_TIDY)
  file(GLOB SRCS "*.cpp")
else ()
  file(GLOB SRCS 
    "../../../external/ctags/libreadtags/*.c"
    "*.cpp")
endif ()

add_library(
  wex-ctags
  ${SRCS})
