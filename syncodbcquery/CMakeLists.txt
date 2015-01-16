project(syncodbcquery)
cmake_minimum_required(VERSION 2.8.7)

if (NOT ${ODBC_FOUND})
  message(FATAL_ERROR "odbc lib not found")
endif()

include_directories(../extension/include)

FILE(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif()
  
add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})
  
target_link_libraries(
  ${PROJECT_NAME}
  wxex-rep
  wxex
  ${wxWidgets_LIBRARIES} 
  ${ODBC_LIBRARIES}
  )
