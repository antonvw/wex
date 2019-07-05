project(syncodbcquery)

include(../../cmake/common.cmake)

if (NOT ${ODBC_FOUND})
  message(FATAL_ERROR "odbc lib not found")
endif()

file(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif()
  
add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})
  
target_link_all(${ODBC_LIBRARIES})
  
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

pack()
