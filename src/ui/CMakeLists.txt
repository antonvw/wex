if(wexBUILD_TIDY)
  file(GLOB SRCS "*.cpp")
else()
  file(
    GLOB SRCS
    "*.cpp"
    "../../external/wxMaterialDesignArtProvider/MaterialDesign/*.cpp"
  )
endif()

add_library(wex-ui ${SRCS})

add_subdirectory(ctags)
