////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class wex::indicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/indicator.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <easylogging++.h>

wex::indicator::indicator(const pugi::xml_node& node)
{
  if (node.empty()) return;

  try
  {
    const auto single = 
      lexers::Get()->ApplyMacro(node.attribute("no").value());

    m_No = std::stoi(single);

    tokenizer fields(node.text().get(), ",");

    const auto style = lexers::Get()->ApplyMacro(fields.GetNextToken());

    m_Style = std::stoi(style);

    if (fields.HasMoreTokens())
    {
      m_ForegroundColour = lexers::Get()->ApplyMacro(fields.GetNextToken());

      if (fields.HasMoreTokens())
      {
        m_Under = (fields.GetNextToken() == "true");
      }
    }

    if (!is_ok())
    {
      log("illegal indicator number:") << m_No << node;
    }
  }
  catch (std::exception& e)
  {
    VLOG(9) << "indicator exception: " << e.what();
  }
}

wex::indicator::indicator(int no, int style)
  : m_No(no)
  , m_Style(style)
{
}

bool wex::indicator::operator<(const wex::indicator& i) const
{
  return m_No < i.m_No;
}

bool wex::indicator::operator==(const wex::indicator& i) const
{
  return m_Style == -1 ?
    m_No == i.m_No:
    m_No == i.m_No && m_Style == i.m_Style;
}

void wex::indicator::Apply(stc* stc) const
{
  if (is_ok())
  {
    stc->IndicatorSetStyle(m_No, m_Style);

    if (!m_ForegroundColour.empty())
    {
      stc->IndicatorSetForeground(m_No, wxString(m_ForegroundColour));
    }

    stc->IndicatorSetUnder(m_No, m_Under);
  }
}

bool wex::indicator::is_ok() const
{
  return 
    m_No >= 0 && m_No <= wxSTC_INDIC_MAX &&
    m_Style >= 0 && m_Style <= wxSTC_INDIC_ROUNDBOX;
}
