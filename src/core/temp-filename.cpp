////////////////////////////////////////////////////////////////////////////////
// Name:      temp-filename.cpp
// Purpose:   Implementation of class wex::temp_filename
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <wex/log.h>
#include <wex/path.h>
#include <wex/temp-filename.h>

wex::temp_filename::temp_filename(bool cleanup)
  : m_cleanup(cleanup)
  , m_name(
      path(
        std::filesystem::temp_directory_path(),
        std::to_string(std::time(nullptr)))
        .data()
        .string() +
      std::to_string(m_no++))
{
}

wex::temp_filename::~temp_filename()
{
  if (m_cleanup && remove(m_name.c_str()) != 0)
  {
    log("could not remove file") << m_name;
  }
}
