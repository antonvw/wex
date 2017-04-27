////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.cpp
// Purpose:   Implementation of wxExSTCData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/stc-data.h>
#include <wx/extension/indicator.h>
#include <wx/extension/stc.h>

wxExSTCData::wxExSTCData(wxExSTC* stc)
  : m_STC(stc)
{
}
  
wxExSTCData::wxExSTCData(wxExSTC* stc, const wxExSTCData& r)
  : m_STC(stc)
{
  *this = r;
}
  
wxExSTCData& wxExSTCData::operator=(const wxExSTCData& r)
{
  if (this != &r)
  {
    m_Col = r.m_Col;
    m_Command = r.m_Command;
    m_Find = r.m_Find;
    m_Line = r.m_Line;
    m_MenuFlags = r.m_MenuFlags;
    m_WinFlags = r.m_WinFlags;

    if (m_STC != nullptr && r.m_STC != nullptr)
    {
      m_STC = r.m_STC;
    }
  }
  
  return *this;
}
  
wxExSTCData& wxExSTCData::Col(int col)
{
  m_Col = col;
  return *this;
}
  
wxExSTCData& wxExSTCData::Command(const std::string& command)
{
  m_Command = command;
  return *this;
}
  
wxExSTCData& wxExSTCData::Find(const std::string& text) 
{
  m_Find = text;
  return *this;
}

wxExSTCData& wxExSTCData::Flags(
  wxExSTCWindowFlags flags, wxExDataAction action)
{
  return Flags<wxExSTCWindowFlags>(flags, m_WinFlags, action);
}
  
bool wxExSTCData::Inject() const
{
  if (m_STC == nullptr) return false;
  
  bool injected = false;
  
  if (m_Line != DATA_INT_NOT_SET)
  {
    InjectLine();
    injected = true;
  }
  
  if (m_Col != DATA_INT_NOT_SET)
  {
    InjectCol();
    injected = true;
  }
  
  if (!m_Find.empty())
  {
    InjectFind();
    injected = true;
  }

  if (!m_Command.empty())
  {
    m_STC->GetVi().Command(m_Command);
    injected = true;
  }
  
  if ((m_WinFlags & STC_WIN_READ_ONLY) ||
      (m_STC->GetFileName().FileExists() && m_STC->GetFileName().IsReadOnly()))
  {
    m_STC->SetReadOnly(true);
    injected = true;
  }

  if (m_STC->GetHexMode().Set((m_WinFlags & STC_WIN_HEX) > 0))
  {
    injected = true;
  }
  
  if (injected)
  {
    m_STC->PropertiesMessage();
  }
  
  return injected;
}
  
void wxExSTCData::InjectCol() const
{
  const int max = (m_Line > 0) ? m_STC->GetLineEndPosition(m_Line - 1): 0;
  const int asked = m_STC->GetCurrentPos() + m_Col - 1;
      
  m_STC->SetCurrentPos(asked < max ? asked: max);
      
  // Reset selection, seems necessary.
  m_STC->SelectNone();
}

void wxExSTCData::InjectFind() const
{
  if (m_Line > 0)
  {
    const int start_pos = m_STC->PositionFromLine(m_Line -1);
    const int end_pos = m_STC->GetLineEndPosition(m_Line -1);

    m_STC->SetSearchFlags(-1);
    m_STC->SetTargetStart(start_pos);
    m_STC->SetTargetEnd(end_pos);

    if (m_STC->SearchInTarget(m_Find) != -1)
    {
      m_STC->SetSelection(m_STC->GetTargetStart(), m_STC->GetTargetEnd());
    }
  }
  else if (m_Line == 0)
  {
    m_STC->FindNext(m_Find, 0);
  }
  else
  {
    m_STC->FindNext(m_Find, 0, false);
  }
}
  
void wxExSTCData::InjectLine() const
{
  if (m_Line > 0)
  {
    m_STC->GotoLine(m_Line -1);
    m_STC->EnsureVisible(m_Line -1);
    m_STC->EnsureCaretVisible();
    
    m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
    m_STC->SetIndicator(wxExIndicator(0), 
      m_STC->PositionFromLine(m_Line -1), 
      m_Col > 0 ? 
        m_STC->PositionFromLine(m_Line -1) + m_Col - 1:
        m_STC->GetLineEndPosition(m_Line -1));
  }
  else
  {
    m_STC->DocumentEnd();
  }
}

wxExSTCData& wxExSTCData::Line(int line)
{
  m_Line = ValidLine(line);
  return *this;
}
  
wxExSTCData& wxExSTCData::Menu(
  wxExSTCMenuFlags flags, wxExDataAction action)
{
  return Flags<wxExSTCMenuFlags>(flags, m_MenuFlags, action);
}

int wxExSTCData::ValidLine(int line) const
{
  if (m_STC != nullptr)
  {
    if (line > m_STC->GetLineCount())
    {
      return m_STC->GetLineCount();
    }
    else if (line < 0)
    {
      return 1;
    }
  }
  
  return line;
}
  
wxExWindowData& wxExWindowData::Id(wxWindowID id) 
{
  m_Id = id;
  return *this;
}
  
wxExWindowData& wxExWindowData::Style(long style) 
{
  m_Style = style;
  return *this;
}
