project(wex-external-lib)

# prevent pango error not finding hb.h
if (UNIX AND NOT (APPLE))
  include_directories("/usr/include/harfbuzz")
endif ()

include_directories("../include")

add_definitions(-D SCI_NAMESPACE)

add_subdirectory(wxWidgets)

add_custom_target(ctags-build
  COMMAND ./autogen.sh
  COMMAND ./configure
  COMMAND make
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/ctags
  COMMENT "ctags build" VERBATIM)

add_custom_target(ctags-install
  COMMAND make install
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/ctags
  COMMENT "ctags install" VERBATIM)

if (MSVC)
  # for appveyor to be able build wxw with wex-core only
  add_custom_target(wxw)
  add_dependencies(wxw
    wxadv   wxaui   wxbase   wxcore   wxgl   wxhtml   wxjpeg   wxlexilla
    wxnet   wxpng   wxqa   wxscintilla   wxstc)
endif ()
