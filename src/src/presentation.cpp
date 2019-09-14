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

    m_no = std::stoi(single);
    m_style = std::stoi(lexers::get()->apply_macro(fields.get_next_token()));

    if (fields.has_more_tokens())
    {
      m_foreground_colour = lexers::get()->apply_macro(fields.get_next_token());

      if (m_type == INDICATOR && fields.has_more_tokens())
      {
        m_under = (fields.get_next_token() == "true");
      }
      
      if (m_type == MARKER && fields.has_more_tokens())
      {
        m_background_colour = lexers::get()->apply_macro(fields.get_next_token());
      }
    }

    if (!is_ok())
    {
      log("illegal " + name() + " number:") << m_no << node;
    }
  }
  catch (std::exception& e)
  {
    log::verbose(e) << name();
  }
}

wex::presentation::presentation(presentation_t type, int no, int style)
  : m_type(type)
  , m_no(no)
  , m_style(style)
{
}

bool wex::presentation::operator<(const wex::presentation& i) const
{
  return m_no < i.m_no;
}

bool wex::presentation::operator==(const wex::presentation& i) const
{
  return m_style == -1 ?
    m_no == i.m_no:
    m_no == i.m_no && m_style == i.m_style;
}

void wex::presentation::apply(stc* stc) const
{
  if (is_ok())
  {
    switch (m_type)
    {
      case INDICATOR:
        stc->IndicatorSetStyle(m_no, m_style);

        if (!m_foreground_colour.empty())
        {
          stc->IndicatorSetForeground(m_no, wxString(m_foreground_colour));
        }

        stc->IndicatorSetUnder(m_no, m_under);
      break;
      
      case MARKER:
        stc->MarkerDefine(m_no, 
          m_style, 
          wxString(m_foreground_colour), 
          wxString(m_background_colour));
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
        m_no >= 0 && m_no <= wxSTC_INDIC_MAX &&
        m_style >= 0 && m_style <= wxSTC_INDIC_ROUNDBOX;
    break;
  
    case MARKER:
      return 
        m_no >= 0 && m_no <= wxSTC_MARKER_MAX &&
        ((m_style >= 0 && m_style <= wxSTC_MARKER_MAX) || 
         (m_style >= wxSTC_MARK_CHARACTER && m_style <= wxSTC_MARK_CHARACTER + 255));
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
