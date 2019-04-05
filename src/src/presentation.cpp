////////////////////////////////////////////////////////////////////////////////
// Name:      presentation.cpp
// Purpose:   Implementation of class wex::presentation
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/presentation.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>

wex::presentation::presentation(
  presentation_t type, const pugi::xml_node& node)
  : m_type(type)
{
  if (node.empty()) return;

  try
  {
    const auto single = 
      lexers::get()->apply_macro(node.attribute("no").value());

    tokenizer fields(node.text().get(), ",");

    m_No = std::stoi(single);
    m_Style = std::stoi(lexers::get()->apply_macro(fields.get_next_token()));

    if (fields.has_more_tokens())
    {
      m_ForegroundColour = lexers::get()->apply_macro(fields.get_next_token());

      if (m_type == INDICATOR && fields.has_more_tokens())
      {
        m_Under = (fields.get_next_token() == "true");
      }
      
      if (m_type == MARKER && fields.has_more_tokens())
      {
        m_BackgroundColour = lexers::get()->apply_macro(fields.get_next_token());
      }
    }

    if (!is_ok())
    {
      log("illegal " + name() + " number:") << m_No << node;
    }
  }
  catch (std::exception& e)
  {
    log::verbose(e) << name();
  }
}

wex::presentation::presentation(presentation_t type, int no, int style)
  : m_type(type)
  , m_No(no)
  , m_Style(style)
{
}

bool wex::presentation::operator<(const wex::presentation& i) const
{
  return m_No < i.m_No;
}

bool wex::presentation::operator==(const wex::presentation& i) const
{
  return m_Style == -1 ?
    m_No == i.m_No:
    m_No == i.m_No && m_Style == i.m_Style;
}

void wex::presentation::apply(stc* stc) const
{
  if (is_ok())
  {
    switch (m_type)
    {
      case INDICATOR:
        stc->IndicatorSetStyle(m_No, m_Style);

        if (!m_ForegroundColour.empty())
        {
          stc->IndicatorSetForeground(m_No, wxString(m_ForegroundColour));
        }

        stc->IndicatorSetUnder(m_No, m_Under);
      break;
      
      case MARKER:
        stc->MarkerDefine(m_No, 
          m_Style, 
          wxString(m_ForegroundColour), 
          wxString(m_BackgroundColour));
      break;
    }
  }
}

bool wex::presentation::is_ok() const
{
  switch (m_type)
  {
    case INDICATOR:
      return 
        m_No >= 0 && m_No <= wxSTC_INDIC_MAX &&
        m_Style >= 0 && m_Style <= wxSTC_INDIC_ROUNDBOX;
    break;
  
    case MARKER:
      return 
        m_No >= 0 && m_No <= wxSTC_MARKER_MAX &&
        ((m_Style >= 0 && m_Style <= wxSTC_MARKER_MAX) || 
         (m_Style >= wxSTC_MARK_CHARACTER && m_Style <= wxSTC_MARK_CHARACTER + 255));
    break;
  }
  
  return false;
}

const std::string wex::presentation::name() const
{
  switch (m_type)
  {
    case INDICATOR: return "indicator"; break;
    case MARKER: return "marker"; break;
  }
  
  return std::string();
}
