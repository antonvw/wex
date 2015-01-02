FILE(GLOB SRCS "*.cpp")
  
add_executable(
  ${PROJECT_NAME} 
  ${SRCS})

target_link_libraries(
  ${PROJECT_NAME} 
  wxex
  ${wxWidgets_LIBRARIES} 
  ${CPPUNIT_LIBRARIES}
  )
