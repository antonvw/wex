if (BUILD_TEST)
  include_directories(../include/)
  
  add_subdirectory(base)
  add_subdirectory(gui)
  add_subdirectory(gui-report)
endif()
