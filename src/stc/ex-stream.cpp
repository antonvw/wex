////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex-stream.h>
#include <wex/stc.h>

wex::ex_stream::ex_stream(wex::stc* stc)
  : m_stc(stc)
  , m_max_line(1000)
{
}

bool wex::ex_stream::find(
  const std::string& text,
  int                find_flags,
  bool               find_next)
{
  return false;
}

bool wex::ex_stream::get_next_line()
{
  if (!std::getline(*m_stream, m_current_line))
  {
    return false;
  }

  m_line++;
  
  if (m_line > m_max_line)
  {
    m_max_line = m_line;
  }
  
  return true;
}

void wex::ex_stream::line(int no)
{
  while (no >= m_line - 1)
  {
    if (!get_next_line())
    { 
      return;
    }
  }
  
  set_text();
}

void wex::ex_stream::set_text()
{
  m_stc->SetReadOnly(false);
  m_stc->SetText(m_current_line);
  m_stc->SetReadOnly(true);
}

void wex::ex_stream::stream(std::fstream& fs)
{
  m_stream = &fs;

  if (get_next_line())
  {
    set_text();
  }
}
