project(wex-test-app)

add_test(NAME ${PROJECT_NAME} COMMAND ${CMAKE_SOURCE_DIR}/test/app/wex-test-app.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/app)
