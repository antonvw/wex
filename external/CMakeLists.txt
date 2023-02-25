project(wex-external-lib)

execute_process(
  COMMAND git apply wxWidgets.patch
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/wxWidgets)

execute_process(
  COMMAND git apply lexilla.patch
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/wxWidgets/src/stc/lexilla)

# prevent pango error not finding hb.h
if (UNIX AND NOT (APPLE))
  include_directories("/usr/include/harfbuzz")
endif ()

include_directories("../include")

add_definitions(-D SCI_NAMESPACE)

add_subdirectory(wxWidgets)
