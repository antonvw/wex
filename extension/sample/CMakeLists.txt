project(wxExSample)

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
  ${wxWidgets_LIBRARIES} 
  wxex
  )

add_subdirectory(report)
