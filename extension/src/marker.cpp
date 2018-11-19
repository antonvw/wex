////////////////////////////////////////////////////////////////////////////////
// Name:      marker.cpp
// Purpose:   Implementation of class wex::marker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/marker.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <easylogging++.h>

wex::marker::marker(const pugi::xml_node& node)
{
  if (node.empty()) return;

  const auto single = 
    lexers::get()->apply_macro(node.attribute("no").value());

  try
  {
    tokenizer fields(node.text().get(), ",");

    m_No = std::stoi(single);
    m_Symbol = std::stoi(lexers::get()->apply_macro(fields.get_next_token()));

    if (fields.has_more_tokens())
    {
      m_ForegroundColour = lexers::get()->apply_macro(fields.get_next_token());

      if (fields.has_more_tokens())
      {
        m_BackgroundColour = lexers::get()->apply_macro(fields.get_next_token());
      }
    }

    if (!is_ok())
    {
      log() << "illegal marker:" << m_No << node;
    }
  }
  catch (std::exception& e)
  {
    VLOG(9) << "marker exception: " << e.what() << ": " << single;
  }
}

wex::marker::marker(int no, int symbol)
  : m_No(no)
  , m_Symbol(symbol)
{
}

bool wex::marker::operator<(const marker& m) const
{
  return m_No < m.m_No;
}

bool wex::marker::operator==(const marker& m) const
{
  return m_Symbol == -1 ? 
    m_No == m.m_No: 
    m_No == m.m_No && m_Symbol == m.m_Symbol;
}

void wex::marker::apply(stc* stc) const
{
  if (is_ok())
  {
    stc->MarkerDefine(m_No, 
      m_Symbol, 
      wxString(m_ForegroundColour), 
      wxString(m_BackgroundColour));
  }
}

bool wex::marker::is_ok() const
{
  return 
    m_No >= 0 && m_No <= wxSTC_MARKER_MAX &&
    ((m_Symbol >= 0 && m_Symbol <= wxSTC_MARKER_MAX) || 
     (m_Symbol >= wxSTC_MARK_CHARACTER && m_Symbol <= wxSTC_MARK_CHARACTER + 255));
}
