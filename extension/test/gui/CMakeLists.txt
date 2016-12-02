project(wxex-test-gui)

FILE(GLOB SRCS "*.cpp" "../*.cpp")
  
add_executable(
  ${PROJECT_NAME} 
  ${SRCS})

if (ODBC_FOUND)
  target_link_all(${ODBC_LIBRARIES})
else ()
  target_link_all()
endif()

if (WIN32)
  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} -e)
else ()
  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
endif ()
