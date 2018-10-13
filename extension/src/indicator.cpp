////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class wex::indicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stc/stc.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/tokenizer.h>
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

    if (!IsOk())
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

void wex::indicator::Apply(wxStyledTextCtrl* stc) const
{
  if (IsOk())
  {
    stc->IndicatorSetStyle(m_No, m_Style);

    if (!m_ForegroundColour.empty())
    {
      stc->IndicatorSetForeground(m_No, wxString(m_ForegroundColour));
    }

    stc->IndicatorSetUnder(m_No, m_Under);
  }
}

bool wex::indicator::IsOk() const
{
  return 
    m_No >= 0 && m_No <= wxSTC_INDIC_MAX &&
    m_Style >= 0 && m_Style <= wxSTC_INDIC_ROUNDBOX;
}
