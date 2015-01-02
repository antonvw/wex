FILE(GLOB SRCS "*.cpp")
  
add_executable(
  ${PROJECT_NAME} 
  ${SRCS})

target_link_libraries(
  ${PROJECT_NAME} 
  ${wxWidgets_LIBRARIES} 
  ${CPPUNIT_LIBRARIES}
  )
