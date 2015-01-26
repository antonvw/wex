project(wxex-test-gui)

FILE(GLOB SRCS "*.cpp" "../*.cpp")
  
add_executable(
  ${PROJECT_NAME} 
  ${SRCS})

if (ODBC_FOUND)
  target_link_libraries(
    ${PROJECT_NAME} 
    wxex
    ${wxWidgets_LIBRARIES} 
    ${CPPUNIT_LIBRARIES}
    ${ODBC_LIBRARY}
    )
else (ODBC_FOUND)
  target_link_libraries(
    ${PROJECT_NAME} 
    wxex
    ${wxWidgets_LIBRARIES} 
    ${CPPUNIT_LIBRARIES}
    )
endif()

install(FILES "../../data/lexers.xml" DESTINATION ~/.${PROJECT_NAME})

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
