////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex-stream.h>
#include <wex/log.h>
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

int wex::ex_stream::get_line_count() const 
{
  return m_last_line_no; 
}

bool wex::ex_stream::get_next_line()
{
  if (!std::getline(*m_stream, m_current_line))
  {
    log::debug("no next line") << m_last_line_no << m_line_no;
    return false;
  }

  m_line_no++;
  
  if (m_line_no > m_last_line_no)
  {
    m_last_line_no = m_line_no + 10;
  }
  
  return true;
}

void wex::ex_stream::goto_line(int no)
{
  if (no < 0 || no < m_line_no)
  {
    if (no < 0)
    {
      no = m_line_no + 1;
    }
    else
    {
      no = 1;
      // currently reset.
      m_line_no = 0;
      m_stream->seekg(0);
    }
  }
  
  while (no > m_line_no)
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
  m_stc->EmptyUndoBuffer();
  m_stc->SetSavePoint();
  m_stc->SetReadOnly(true);
  m_stc->use_modification_markers(false);
}

void wex::ex_stream::stream(std::fstream& fs)
{
  m_stream = &fs;

  if (get_next_line())
  {
    set_text();
  }
}
