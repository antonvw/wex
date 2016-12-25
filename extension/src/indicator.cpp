////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class wxExIndicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexers.h>
#include <wx/extension/tokenizer.h>

wxExIndicator::wxExIndicator(const pugi::xml_node& node)
{
  if (node.empty()) return;

  const std::string single = 
    wxExLexers::Get()->ApplyMacro(node.attribute("no").value());

  try
  {
    m_No = std::stoi(single);
  }
  catch (std::exception& e)
  {
    std::cerr << "Illegal indicator: " << single << " with offset: " << 
      node.offset_debug() << "\n";
    return;
  }

  wxExTokenizer fields(node.text().get(), ",");

  const std::string style = wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

  try
  {
    m_Style = std::stoi(style);

    if (fields.HasMoreTokens())
    {
      m_ForegroundColour = wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

      if (fields.HasMoreTokens())
      {
        m_Under = (fields.GetNextToken() == "true");
      }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Illegal indicator style: " << style << " with offset: " << 
      node.offset_debug() << "\n";
  }

  if (!IsOk())
  {
    std::cerr << "Illegal indicator number: " << m_No << " with offset: " << 
      node.offset_debug() << "\n";
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
