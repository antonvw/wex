project(syncped)

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
  )
  
if (CMAKE_BUILD_TYPE MATCHES "Release")
  if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND ../../bin/deploy.bat)
  else ()
    add_custom_command(TARGET ${PROJECT_NAME}
      POST_BUILD
      COMMAND ../../bin/deploy.sh)
  endif ()
endif ()
