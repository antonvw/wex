////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wex::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/dir-data.h>

wex::data::dir& wex::data::dir::dir_spec(const std::string& rhs)
{
  m_dir_spec = rhs;

  return *this;
}

wex::data::dir& wex::data::dir::file_spec(const std::string& rhs)
{
  m_file_spec = rhs;

  return *this;
}

wex::data::dir& wex::data::dir::max_matches(int rhs)
{
  m_max_matches = rhs;

  return *this;
}

wex::data::dir& wex::data::dir::type(type_t rhs)
{
  m_flags = rhs;

  return *this;
}
