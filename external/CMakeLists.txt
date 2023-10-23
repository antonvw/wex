project(wex-external-lib)

# prevent pango error not finding hb.h
if (UNIX AND NOT (APPLE))
  include_directories("/usr/include/harfbuzz")
endif ()

include_directories("../include")

add_definitions(-D SCI_NAMESPACE)

add_subdirectory(wxWidgets)

add_custom_target(ctags
  COMMAND ./autogen.sh
  COMMAND ./configure
  COMMAND make
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/ctags
  COMMENT "ctags" VERBATIM)
  
add_custom_target(ctags-install
  COMMAND make install
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/ctags
  COMMENT "ctags install" VERBATIM)
