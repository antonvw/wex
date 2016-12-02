project(wxex-test-base)

FILE(GLOB SRCS "*.cpp" "../*.cpp")
  
add_executable(
  ${PROJECT_NAME} 
  ${SRCS})

target_link_all()
  
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
