////////////////////////////////////////////////////////////////////////////////
// Name:      stc/find.cpp
// Purpose:   Implementation of class wex::stc find methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wex/config.h>
#include <wex/find-data.h>
#include <wex/path.h>
#include <wex/stc-core.h>

wex::data::find::find(wex::core::stc* stc, bool forward)
  : m_stc(stc)
  , m_forward(forward)
{
  set_pos();
}

wex::data::find& wex::data::find::flags(int rhs)
{
  m_flags = rhs;
  return *this;
}

bool wex::data::find::find_margin(const std::string& text, int& found_line)
{
  const bool wrapscan(config(_("stc.Wrap scan")).get(true));
  std::match_results<std::string::const_iterator> m;

  int  line  = m_stc->LineFromPosition(m_start_pos) + (m_forward ? +1 : -1);
  bool found = false;

  do
  {
    if (const std::string margin(m_stc->MarginGetText(line));
        ((m_flags & wxSTC_FIND_REGEXP) &&
         std::regex_search(margin, m, std::regex(text))) ||
        margin.find(text) != std::string::npos)
    {
      found_line = line;
      found      = true;
    }
    else
    {
      m_forward ? line++ : line--;
    }
  } while (((m_forward && line <= m_stc->LineFromPosition(m_end_pos)) ||
            (!m_forward && line >= m_stc->LineFromPosition(m_end_pos))) &&
           !found);

  if (!found && !m_recursive && wrapscan)
  {
    m_recursive = true;

    set_pos();
    found = find_margin(text, line);

    m_recursive = false;
  }

  return found;
}

wex::data::find& wex::data::find::set_pos()
{
  if (m_forward)
  {
    if (m_recursive)
    {
      m_start_pos = 0;
      m_end_pos   = m_stc->GetCurrentPos();
    }
    else
    {
      m_start_pos = m_stc->GetCurrentPos();
      m_end_pos   = m_stc->GetTextLength();
    }
  }
  else
  {
    if (m_recursive)
    {
      m_start_pos = m_stc->GetTextLength();
      m_end_pos   = m_stc->GetCurrentPos();

      if (m_stc->GetSelectionStart() != -1)
        m_end_pos = m_stc->GetSelectionStart();
    }
    else
    {
      m_start_pos = m_stc->GetCurrentPos();

      if (m_stc->GetSelectionStart() != -1)
        m_start_pos = m_stc->GetSelectionStart();

      m_end_pos = 0;
    }
  }

  return *this;
}
