project(syncodbcquery)

if (NOT ${ODBC_FOUND})
  message(FATAL_ERROR "odbc lib not found")
endif()

include_directories(../extension/include)
include_directories(../extension/pugixml/src)

FILE(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif()
  
add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})
  
target_link_all(${ODBC_LIBRARIES})
  
add_subdirectory(locale)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

pack()
