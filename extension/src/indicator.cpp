////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class wxExIndicator
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
#include <wx/extension/tokenizer.h>
#include <easylogging++.h>

wxExIndicator::wxExIndicator(const pugi::xml_node& node)
{
  if (node.empty()) return;

  try
  {
    const std::string single = 
      wxExLexers::Get()->ApplyMacro(node.attribute("no").value());

    m_No = std::stoi(single);

    wxExTokenizer fields(node.text().get(), ",");

    const std::string style = wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

    m_Style = std::stoi(style);

    if (fields.HasMoreTokens())
    {
      m_ForegroundColour = wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

      if (fields.HasMoreTokens())
      {
        m_Under = (fields.GetNextToken() == "true");
      }
    }

    if (!IsOk())
    {
      LOG(ERROR) << "illegal indicator number: " << m_No << " with offset: " << node.offset_debug();
    }
  }
  catch (std::exception& e)
  {
    LOG(ERROR) << "indicator exception: " << e.what();
  }
}

wxExIndicator::wxExIndicator(int no, int style)
  : m_No(no)
  , m_Style(style)
{
}

bool wxExIndicator::operator<(const wxExIndicator& i) const
{
  return m_No < i.m_No;
}

bool wxExIndicator::operator==(const wxExIndicator& i) const
{
  return m_Style == -1 ?
    m_No == i.m_No:
    m_No == i.m_No && m_Style == i.m_Style;
}

void wxExIndicator::Apply(wxStyledTextCtrl* stc) const
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

bool wxExIndicator::IsOk() const
{
  return 
    m_No >= 0 && m_No <= wxSTC_INDIC_MAX &&
    m_Style >= 0 && m_Style <= wxSTC_INDIC_ROUNDBOX;
}
