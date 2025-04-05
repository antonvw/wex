////////////////////////////////////////////////////////////////////////////////
// Name:      data/find.cpp
// Purpose:   Implementation of class wex::data::find
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/data/find.h>
#include <wex/factory/stc.h>

#include <boost/regex.hpp>

wex::data::find::find(const std::string& text, bool forward)
  : m_text(text)
  , m_forward(forward)
{
}

wex::data::find::find(
  wex::factory::stc* stc,
  const std::string& text,
  bool               forward)
  : m_stc(stc)
  , m_text(text)
  , m_forward(forward)
{
  set_pos();
}

wex::data::find::find(const std::string& text, int line, int pos, bool forward)
  : m_text(text)
  , m_line_no(line)
  , m_pos(pos)
  , m_forward(forward)
{
}

bool wex::data::find::find_margin(int& found_line)
{
  assert(m_stc != nullptr);

  const bool wrapscan(config(_("stc.Wrap scan")).get(true));
  boost::match_results<std::string::const_iterator> m;

  int line = !m_recursive ?
               m_stc->LineFromPosition(m_start_pos) + (m_forward ? +1 : -1) :
               m_stc->LineFromPosition(m_start_pos);

  bool found = false;

  do
  {
    if (const std::string margin(m_stc->MarginGetText(line));
        ((m_flags & wxSTC_FIND_REGEXP) &&
         boost::regex_search(margin, m, boost::regex(m_text))) ||
        margin.contains(m_text))
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
    statustext();

    m_recursive = true;

    set_pos();
    found = find_margin(line);

    if (found)
    {
      found_line = line;
    }
    else
    {
      statustext();
    }

    m_recursive = false;
  }

  return found;
}

wex::data::find& wex::data::find::flags(int rhs)
{
  m_flags = rhs;
  return *this;
}

const std::string wex::data::find::get_find_result() const
{
  if (!recursive())
  {
    const auto where =
      (is_forward()) ? _("bottom").ToStdString() : _("top").ToStdString();

    return _("Searching for").ToStdString() + " " +
           quoted(boost::algorithm::trim_copy(text())) + " " +
           _("hit").ToStdString() + " " + where;
  }

  if (config("ex-set.errorbells").get(true))
  {
    wxBell();
  }

  return quoted(boost::algorithm::trim_copy(text())) + " " +
         _("not found").ToStdString();
}

void wex::data::find::set_pos()
{
  assert(m_stc != nullptr);

  if (m_forward)
  {
    if (m_recursive)
    {
      m_start_pos = 0;
      m_end_pos   = m_stc->GetCurrentPos();

      if (m_stc->GetTargetStart() == m_stc->GetTargetEnd())
      {
        m_end_pos++;
      }
    }
    else
    {
      m_start_pos = m_stc->GetCurrentPos();
      m_end_pos   = m_stc->GetTextLength();

      if (
        m_stc->GetTargetStart() == m_stc->GetTargetEnd() &&
        m_stc->GetTargetStart() != -1)
      {
        m_start_pos++;
      }
    }
  }
  else
  {
    if (m_recursive)
    {
      m_start_pos = m_stc->GetTextLength();
      m_end_pos   = m_stc->GetCurrentPos();

      if (m_stc->GetSelectionStart() != -1)
      {
        m_end_pos = m_stc->GetSelectionStart();
      }
    }
    else
    {
      m_start_pos = m_stc->GetCurrentPos();

      if (m_stc->GetSelectionStart() != -1)
      {
        m_start_pos = m_stc->GetSelectionStart();
      }

      m_end_pos = 0;
    }
  }
}

void wex::data::find::statustext() const
{
  log::status(get_find_result());
}
