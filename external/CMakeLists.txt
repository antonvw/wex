project(wex-external-lib)

# prevent pango error not finding hb.h
if (UNIX AND NOT (APPLE))
  include_directories("/usr/include/harfbuzz")
endif ()

add_subdirectory(wxWidgets)
