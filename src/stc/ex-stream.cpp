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
{
}

bool wex::ex_stream::find(
  const std::string& text,
  int                find_flags,
  bool               find_next)
{
  return false;
}

std::string wex::ex_stream::get_line() const
{
  return std::string();
}

void wex::ex_stream::line(int no)
{
  m_line = no;
}

void wex::ex_stream::line_next()
{
  m_line++;
  
  std::string line;
  std::getline(*m_stream, line);

  m_stc->SetText(line);
}

void wex::ex_stream::stream(std::fstream& fs)
{
  m_stream = &fs;

  line_next();
}
