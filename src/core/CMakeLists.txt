if(wexBUILD_TIDY)
  file(GLOB SRCS "*.cpp")
else()
  file(GLOB SRCS "*.cpp" "../../external/pugixml/src/*.cpp")
endif()

add_library(wex-core ${SRCS})
