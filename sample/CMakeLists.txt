project(wex-sample)

include(../cmake/common.cmake)

file(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif(WIN32)

add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})

wex_target_link_all()
