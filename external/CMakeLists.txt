project(wex-external-lib)

add_definitions(-DELPP_NO_DEFAULT_LOG_FILE)

# prevent pango error not finding hb.h
if (UNIX AND NOT (APPLE))
  include_directories("/usr/include/harfbuzz")
endif ()

add_subdirectory(wxWidgets)
