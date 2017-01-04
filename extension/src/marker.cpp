////////////////////////////////////////////////////////////////////////////////
// Name:      marker.cpp
// Purpose:   Implementation of class wxExMarker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stc/stc.h>
#include <wx/extension/marker.h>
#include <wx/extension/lexers.h>
#include <wx/extension/tokenizer.h>

wxExMarker::wxExMarker(const pugi::xml_node& node)
{
  if (node.empty()) return;

  try
  {
    const std::string single = 
      wxExLexers::Get()->ApplyMacro(node.attribute("no").value());

    m_No = std::stoi(single);

    wxExTokenizer fields(node.text().get(), ",");

    const std::string symbol = wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

    m_Symbol = std::stoi(symbol);

    if (fields.HasMoreTokens())
    {
      m_ForegroundColour = wxExLexers::Get()->ApplyMacro(fields.GetNextToken());

      if (fields.HasMoreTokens())
      {
        m_BackgroundColour = wxExLexers::Get()->ApplyMacro(fields.GetNextToken());
      }
    }

    if (!IsOk())
    {
      std::cerr << "Illegal marker: " << m_No << " with offset: " << node.offset_debug() << "\n";
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Marker exception with offset: " << node.offset_debug() << "\n";
  }
}

wxExMarker::wxExMarker(int no, int symbol)
  : m_No(no)
  , m_Symbol(symbol)
{
}

bool wxExMarker::operator<(const wxExMarker& m) const
{
  return m_No < m.m_No;
}

bool wxExMarker::operator==(const wxExMarker& m) const
{
  return m_Symbol == -1 ? 
    m_No == m.m_No: 
    m_No == m.m_No && m_Symbol == m.m_Symbol;
}

void wxExMarker::Apply(wxStyledTextCtrl* stc) const
{
  if (IsOk())
  {
    stc->MarkerDefine(m_No, 
      m_Symbol, 
      wxString(m_ForegroundColour), 
      wxString(m_BackgroundColour));
  }
}

bool wxExMarker::IsOk() const
{
  return 
    m_No >= 0 && m_No <= wxSTC_MARKER_MAX &&
    ((m_Symbol >= 0 && m_Symbol <= wxSTC_MARKER_MAX) || 
     (m_Symbol >= wxSTC_MARK_CHARACTER && m_Symbol <= wxSTC_MARK_CHARACTER + 255));
}
