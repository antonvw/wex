include(
  ${wxWidgets_USE_FILE})

find_package(cppunit OPTIONAL_COMPONENTS )

if (cppunit_FOUND)
  add_subdirectory(base)
  add_subdirectory(gui)
  add_subdirectory(gui-report)
endif()
