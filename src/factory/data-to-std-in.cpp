////////////////////////////////////////////////////////////////////////////////
// Name:      data-to-std-in.cpp
// Purpose:   Implementation of class wex::data_to_std_in
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/file.h>
#include <wex/core/temp-filename.h>

#include "data-to-std-in.h"

wex::data_to_std_in::data_to_std_in(const process_data& data)
  : m_data(data)
{
}

wex::data_to_std_in::~data_to_std_in()
{
  if (m_opened)
  {
    fclose(m_in);
  }
}

FILE* wex::data_to_std_in::std_in()
{
  if (!m_data.std_in().empty())
  {
    const temp_filename tmp(true);
    wex::file(path(tmp.name()), std::ios::out).write(m_data.std_in());
    m_in = fopen(tmp.name().c_str(), "r");

    if (m_in != nullptr)
    {
      m_opened = true;
    }
  }

  return m_in;
}
