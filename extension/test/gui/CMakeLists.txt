include(
  ${wxWidgets_USE_FILE})

set(SRCS 
  main.cpp 
  test.cpp 
  )
  
add_executable(
  ${PROJECT_NAME} 
  ${SRCS})

target_link_libraries(
  ${PROJECT_NAME} 
  wxex
  ${wxWidgets_LIBRARIES} 
  ${CPPUNIT_LIBRARIES}
  )
