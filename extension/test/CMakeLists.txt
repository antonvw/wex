if (BUILD_TEST)
  include_directories(../include/)
  include_directories(doctest/doctest)
  
  add_subdirectory(base)
  add_subdirectory(gui)
  add_subdirectory(gui-report)
endif()
