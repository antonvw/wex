project(wxex-test)

include_directories(../include/)
include_directories(../../external/doctest/doctest)
  
add_subdirectory(base)
add_subdirectory(gui)
add_subdirectory(gui-report)

if (WIN32)
  set(CONFIG_INSTALL_DIR bin)
elseif (APPLE)
  set(CONFIG_INSTALL_DIR /Users/$ENV{USER}/.config/${PROJECT_NAME})
else ()
  set(CONFIG_INSTALL_DIR /home/${user}/.config/${PROJECT_NAME})
endif ()

file(MAKE_DIRECTORY ${CONFIG_INSTALL_DIR})
file(GLOB_RECURSE pattern_files RELATIVE
 "${CMAKE_CURRENT_SOURCE_DIR}/" "../data/*.xml" )

foreach(p ${pattern_files})
  file(COPY ${p} DESTINATION ${CONFIG_INSTALL_DIR})
endforeach()

file(COPY data/.odbc.ini DESTINATION ${CONFIG_INSTALL_DIR})
configure_file(../data/conf.elp ${CONFIG_INSTALL_DIR}/conf.elp)
