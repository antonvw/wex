project(wex-test)

include_directories(../include/)
include_directories(../external/doctest/doctest)
  
if (wexBUILD_SAMPLES)
  add_subdirectory(app)
endif ()

add_subdirectory(core)
add_subdirectory(factory)
add_subdirectory(data-src)
add_subdirectory(common)
add_subdirectory(ui)
add_subdirectory(ex)
add_subdirectory(vi)
add_subdirectory(stc)
add_subdirectory(del)

if (WIN32)
  set(CONFIG_INSTALL_DIR /Users/$ENV{USERNAME}/.config/${PROJECT_NAME})
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
