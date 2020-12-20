////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wex/core.h>
#include <wex/ex-stream.h>
#include <wex/log.h>
#include <wex/managed-frame.h>
#include <wex/stc.h>

wex::ex_stream::ex_stream(wex::stc* stc)
  : m_context_size(500)
  , m_line_size(500)
  , m_current_line(new char[m_line_size])
  , m_stc(stc)
{
}

wex::ex_stream::~ex_stream()
{
  delete[] m_current_line;
}
  
void wex::ex_stream::add_text(const std::string& text) {}

bool wex::ex_stream::find(const std::string& text)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("stream find") << text;

  auto line_no = m_line_no;
  auto pos     = m_stream->tellg();
  bool found   = false;

  const std::regex r(text);

  while (!found && get_next_line())
  {
    if (std::regex_search(m_current_line, r))
    {
      found = true;
    }
  }

  if (!found)
  {
    m_line_no = line_no;
    m_stream->clear();
    m_stream->seekg(pos);

    m_stc->get_frame()->statustext(
      get_find_result(text, true, true),
      std::string());
  }
  else
  {
    log::trace("stream found") << text << m_line_no;
    m_context = m_current_line;
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

  auto pos = m_stream->tellg();
  m_stream->clear();
  m_stream->seekg(0);
  m_last_line_no = 0;
  
  char c;
  while (m_stream->get(c))
  {
    if (c == '\n')
    {
      m_last_line_no++;
    }
  }

  m_stream->clear();
  m_stream->seekg(pos);

  return m_last_line_no;
}

bool wex::ex_stream::get_next_line()
{
  if (!m_stream->getline(m_current_line, m_line_size))
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

  if (no < m_line_no)
  {
    // currently reset.
    m_line_no = -1;
    m_stream->clear();
    m_stream->seekg(0);
    log::trace("stream reset");
    m_context.clear();

    while ((m_line_no < no) && get_next_line())
    {
    }
  }
  else
  {
    while ((no > m_line_no) && get_next_line())
    {
    }
  }

  set_context();
  set_text();
}

void wex::ex_stream::insert_text(int line, const std::string& text) {}

void wex::ex_stream::set_context()
{
  m_context += (!m_context.empty() ? "\n" : std::string()) + m_current_line;

  if (m_context.size() > m_context_size)
  {
    m_context = m_context.substr(strlen(m_current_line));
  }
}

void wex::ex_stream::set_text()
{
  m_stc->SetReadOnly(false);
  m_stc->SetText(m_context);
  m_stc->DocumentEnd();
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
