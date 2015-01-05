project(wxex-sample-report)

include_directories(
  ../include/)

FILE(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif(WIN32)

add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})

target_link_libraries(
  ${PROJECT_NAME}
  wxex-rep
  wxex
  ${wxWidgets_LIBRARIES} 
  )
