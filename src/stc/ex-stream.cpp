////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wex/ex-stream.h>
#include <wex/log.h>
#include <wex/stc.h>

wex::ex_stream::ex_stream(wex::stc* stc)
  : m_stc(stc)
{
}

void wex::ex_stream::add_text(const std::string& text) {}

bool wex::ex_stream::find(
  const std::string& text,
  int                find_flags,
  bool               find_next)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("stream find") << text;

  auto line_no = m_line_no;
  auto pos     = m_stream->tellg();
  bool found   = false;

  std::match_results<std::string::const_iterator> m;

  while (!found && get_next_line())
  {
    if (std::regex_search(m_current_line, m, std::regex(text)))
    {
      found = true;
    }
  }

  if (!found)
  {
    m_line_no = line_no;
    m_stream->clear();
    m_stream->seekg(pos);
  }
  else
  {
    log::trace("stream found") << text << m_line_no;
    set_text();
  }

  return found;
}

int wex::ex_stream::get_current_line() const
{
  return m_line_no;
}

int wex::ex_stream::get_line_count() const
{
  return m_last_line_no;
}

int wex::ex_stream::get_line_count_request()
{
  if (m_stream == nullptr)
  {
    return LINE_COUNT_UNKNOWN;
  }

  while (get_next_line())
  {
  }
  
  m_last_line_no = m_line_no;
  
  return m_line_no;
}
  
bool wex::ex_stream::get_next_line()
{
  if (!std::getline(*m_stream, m_current_line))
  {
    log::debug("no next line") << m_line_no;
    return false;
  }

  m_line_no++;

  return true;
}

void wex::ex_stream::goto_line(int no)
{
  if (m_stream == nullptr)
  {
    return;
  }

  log::trace("stream goto_line") << no << m_line_no;

  if (no <= 0 || no < m_line_no)
  {
    if (no < 0)
    {
      no = m_line_no + 1;
    }
    else
    {
      no = 0;

      // currently reset.
      m_line_no = -1;
      m_stream->clear();
      m_stream->seekg(0);
      log::trace("stream reset");
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

void wex::ex_stream::insert_text(int line, const std::string& text) {}

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

  goto_line(0);
}
