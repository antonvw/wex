////////////////////////////////////////////////////////////////////////////////
// Name:      stc/auto-indent.cpp
// Purpose:   Implementation of class wex::auto_indent
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/stc-undo.h>
#include <wex/stc/auto-indent.h>
#include <wex/stc/stc.h>

wex::auto_indent::auto_indent(wex::stc* stc)
  : m_stc(stc)
{
}

bool wex::auto_indent::determine_indent(int level)
{
  if (m_previous_level != -1 && level > m_previous_level)
  {
    m_indent += m_stc->GetIndent();
  }
  else if (
    m_previous_level != -1 && level < m_previous_level &&
    m_indent >= m_stc->GetIndent())
  {
    m_indent -= m_stc->GetIndent();
  }
  else if (m_previous_level == -1 && level > 0 && m_indent == 0)
  {
    m_indent = level * m_stc->GetIndent();
  }

  if (m_indent == 0)
  {
    return false;
  }

  log::trace("auto_indent::determine_indent") << m_indent;

  return true;
}

bool wex::auto_indent::find_indented_line(int start_line)
{
  const int max = 100;

  for (int line = start_line, i = 0; line >= 0 && i < max; line--, i++)
  {
    if (int indent = m_stc->GetLineIndentation(line); indent > 0)
    {
      m_indent = indent;
      log::trace("auto_indent::find_indented_line")
        << line << "indent" << m_indent;
      return true;
    }
    else if (!m_stc->GetLineText(line).empty())
    {
      log::trace("auto_indent::find_indented_line") << line << "not empty";
      m_indent = 0;
      return false;
    }
  }

  return false;
}

void wex::auto_indent::indent(int line, int level)
{
  stc_undo undo(m_stc);

  m_stc->SetLineIndentation(line, m_indent);

  if (level < m_previous_level && line > 0)
  {
    m_stc->SetLineIndentation(line - 1, m_indent);
  }
}

bool wex::auto_indent::is_indentable(int c) const
{
  bool is_nl = false;

  switch (m_stc->GetEOLMode())
  {
    case wxSTC_EOL_CR:
      is_nl = (c == '\r');
      break;

    case wxSTC_EOL_CRLF:
      is_nl = (c == '\n');
      break; // so ignore first \r

    case wxSTC_EOL_LF:
      is_nl = (c == '\n');
      break;
  }

  return is_nl;
}

bool wex::auto_indent::on_char(int c)
{
  if (!use() || !is_indentable(c))
  {
    return false;
  }

  const auto line = m_stc->get_current_line();

  find_indented_line(line);

  const auto level = m_stc->get_fold_level();

  if (!determine_indent(level))
  {
    m_previous_level = -1;
    return false;
  }

  indent(line, level);

  m_previous_level = level;

  m_stc->GotoPos(m_stc->GetLineIndentPosition(line));

  return true;
}

bool wex::auto_indent::use()
{
  return config(_("stc.Auto indent")).get(true);
}

void wex::auto_indent::use(bool on)
{
  config(_("stc.Auto indent")).set(on);
}
