////////////////////////////////////////////////////////////////////////////////
// Name:      temp-filename.cpp
// Purpose:   Implementation of class wex::temp_filename
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wex/core/temp-filename.h>

#include <filesystem>

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
  if (m_cleanup)
  {
    if (wex::path p(m_name); p.file_exists() && remove(m_name.c_str()) != 0)
    {
      log("could not remove file") << m_name;
    }
  }
}
