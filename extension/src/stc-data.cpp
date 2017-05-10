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

wxExSTCData::wxExSTCData(wxExControlData& data, wxExSTC* stc)
  : m_Data(data)
  , m_STC(stc)
{
}

wxExSTCData::wxExSTCData(wxExWindowData& data, wxExSTC* stc)
  : m_Data(wxExControlData().Window(data))
  , m_STC(stc)
{
}

wxExSTCData& wxExSTCData::operator=(const wxExSTCData& r)
{
  if (this != &r)
  {
    m_Data = r.m_Data;
    m_MenuFlags = r.m_MenuFlags;
    m_WinFlags = r.m_WinFlags;

    if (m_STC != nullptr && r.m_STC != nullptr)
    {
      m_STC = r.m_STC;
    }
  }
  
  return *this;
}
  
wxExSTCData& wxExSTCData::Flags(
  wxExSTCWindowFlags flags, wxExDataAction action)
{
  m_Data.Flags<wxExSTCWindowFlags>(flags, m_WinFlags, action);

  return *this;
}
  
bool wxExSTCData::Inject() const
{
  if (m_STC == nullptr) return false;
  
  bool injected = m_Data.Inject(
    [&]() {
      if (m_Data.Line() > 0)
      {
        m_STC->GotoLine(m_Data.Line() -1);
        m_STC->EnsureVisible(m_Data.Line() -1);
        m_STC->EnsureCaretVisible();
        
        m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
        m_STC->SetIndicator(wxExIndicator(0), 
          m_STC->PositionFromLine(m_Data.Line() -1), 
          m_Data.Col() > 0 ? 
            m_STC->PositionFromLine(m_Data.Line() -1) + m_Data.Col() - 1:
            m_STC->GetLineEndPosition(m_Data.Line() -1));
      }
      else if (m_Data.Line() == DATA_NUMBER_NOT_SET)
      {
        return false;
      }
      else
      {
        m_STC->DocumentEnd();
      }
      return true;},
    [&]() {
      const int max = (m_Data.Line() > 0) ? m_STC->GetLineEndPosition(m_Data.Line() - 1): 0;
      const int asked = m_STC->GetCurrentPos() + m_Data.Col() - 1;
          
      m_STC->SetCurrentPos(asked < max ? asked: max);
          
      // Reset selection, seems necessary.
      m_STC->SelectNone();
      return true;},
    [&]() {
      if (m_Data.Line() > 0)
      {
        const int start_pos = m_STC->PositionFromLine(m_Data.Line() -1);
        const int end_pos = m_STC->GetLineEndPosition(m_Data.Line() -1);

        m_STC->SetSearchFlags(-1);
        m_STC->SetTargetStart(start_pos);
        m_STC->SetTargetEnd(end_pos);

        if (m_STC->SearchInTarget(m_Data.Find()) != -1)
        {
          m_STC->SetSelection(m_STC->GetTargetStart(), m_STC->GetTargetEnd());
        }
      }
      else if (m_Data.Line() == 0)
      {
        m_STC->FindNext(m_Data.Find(), 0);
      }
      else
      {
        m_STC->FindNext(m_Data.Find(), 0, false);
      }
      return true;},
    [&]() {
      return m_STC->GetVi().Command(m_Data.Command());});
  
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
  
wxExSTCData& wxExSTCData::Menu(
  wxExSTCMenuFlags flags, wxExDataAction action)
{
  m_Data.Flags<wxExSTCMenuFlags>(flags, m_MenuFlags, action);

  return *this;
}
