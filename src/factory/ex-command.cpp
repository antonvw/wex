////////////////////////////////////////////////////////////////////////////////
// Name:      ex/command.cpp
// Purpose:   Implementation of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex-command.h>
#include <wex/factory/stc.h>
#include <wx/textentry.h>

wex::ex_command::ex_command() {}

wex::ex_command::ex_command(const std::string& command)
  : m_text(command)
{
}

wex::ex_command::ex_command(wex::factory::stc* stc)
  : m_stc(stc)
  , m_stc_original(stc)
{
}

wex::ex_command::ex_command(const ex_command& c)
{
  *this = c;
}

wex::ex_command& wex::ex_command::operator=(const ex_command& c)
{
  if (this != &c)
  {
    m_has_type = c.m_has_type;
    m_text     = c.m_text;

    if (c.m_stc != nullptr)
    {
      m_stc = c.m_stc;
    }

    if (c.m_stc_original != nullptr)
    {
      m_stc_original = c.m_stc_original;
    }
  }

  return *this;
}

bool wex::ex_command::append_exec(char c)
{
  append(c);
  return exec();
}

void wex::ex_command::erase(size_t pos, size_t len)
{
  if (pos >= m_text.size())
  {
    return;
  }

  if (pos + len >= m_text.size())
  {
    len = std::string::npos;
  }

  m_text.erase(pos + (m_has_type ? str().size() : 0), len);
}

bool wex::ex_command::exec() const
{
  return m_stc != nullptr && m_stc->vi_command(command());
}

void wex::ex_command::handle(const wxTextEntry* te, int keycode)
{
  switch (const size_t pos = te->GetInsertionPoint(); keycode)
  {
    case wxID_CUT:
      if (!te->GetStringSelection().empty())
      {
        long from, to;
        te->GetSelection(&from, &to);
        erase(from, to - from);
      }
      break;

    case WXK_BACK:
      if (!empty())
      {
        size_t len = 1;

        if (!te->GetStringSelection().empty())
        {
          long from, to;
          te->GetSelection(&from, &to);
          len = to - from;
        }

        if (pos == te->GetLastPosition())
        {
          pop_back();
        }
        else
        {
          erase(pos - 1, len);
        }
      }
      break;

    case WXK_NONE:
      break;

    default:
      if (pos == te->GetLastPosition())
      {
        append(keycode);
      }
      else
      {
        insert(pos, keycode);
      }
  }
}

void wex::ex_command::insert(size_t pos, char c)
{
  if (m_has_type)
  {
    if (pos < m_text.size())
    {
      m_text.insert(str().size() + pos, 1, c);
    }
    else
    {
      append(c);
    }
  }
}

void wex::ex_command::insert(size_t pos, const std::string& s)
{
  if (m_has_type)
  {
    if (pos < m_text.size())
    {
      m_text.insert(str().size() + pos, s);
    }
    else
    {
      append(s);
    }
  }
}

void wex::ex_command::no_type()
{
  m_has_type = false;
}

wex::ex_command& wex::ex_command::reset(const std::string& text, bool full)
{
  m_text = m_has_type ? m_text.substr(0, str().size()) + text : text;

  if (full)
  {
    m_stc          = nullptr;
    m_stc_original = nullptr;
  }

  return *this;
}

void wex::ex_command::restore(const ex_command& c)
{
  m_has_type = c.m_has_type;
  m_text     = c.m_text;

  if (c.m_stc != nullptr || c.m_stc_original != nullptr)
  {
    m_stc = (c.m_stc_original != nullptr ? c.m_stc_original : c.m_stc);
  }
}

wex::ex_command& wex::ex_command::set(const std::string& text)
{
  m_text = text;

  return *this;
}

void wex::ex_command::set(const ex_command& c)
{
  m_has_type = c.m_has_type;
  m_text     = c.m_text;

  if (c.m_stc != nullptr || c.m_stc_original != nullptr)
  {
    m_stc = (c.m_stc == c.m_stc_original ? c.m_stc : c.m_stc_original);
  }
}

std::string wex::ex_command::str() const
{
  if (!m_has_type)
  {
    return m_text;
  }
  else
  {
    switch (type())
    {
      case type_t::NONE:
        return std::string();

      case type_t::CALC:
        return m_text.substr(0, 2);

      default:
        return m_text.substr(0, 1);
    }
  }
}

wex::ex_command::type_t wex::ex_command::type() const
{
  if (m_text.empty() || !m_has_type)
  {
    return type_t::NONE;
  }
  else
    switch (m_text[0])
    {
      case WXK_CONTROL_R:
        return m_text.size() > 1 && m_text[1] == '=' ? type_t::CALC :
                                                       type_t::NONE;

      case ':':
        return m_stc != nullptr && !m_stc->is_visual() ? type_t::COMMAND_EX :
                                                         type_t::COMMAND;

      case '!':
        return type_t::ESCAPE;

      case '/':
      case '?':
        return m_stc != nullptr && m_stc->get_margin_text_click() > 0 ?
                 type_t::FIND_MARGIN :
                 type_t::FIND;

      default:
        return type_t::VI;
    }
}
