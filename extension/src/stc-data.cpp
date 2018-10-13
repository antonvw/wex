////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.cpp
// Purpose:   Implementation of wex::stc_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/stc-data.h>
#include <wx/extension/indicator.h>
#include <wx/extension/stc.h>

wex::stc_data::stc_data(stc* stc)
  : m_STC(stc)
{
}
  
wex::stc_data::stc_data(stc* stc, const stc_data& r)
  : m_STC(stc)
{
  *this = r;
}

wex::stc_data::stc_data(control_data& data, stc* stc)
  : m_Data(data)
  , m_STC(stc)
{
}

wex::stc_data::stc_data(window_data& data, stc* stc)
  : m_Data(control_data().Window(data))
  , m_STC(stc)
{
}

wex::stc_data& wex::stc_data::operator=(const stc_data& r)
{
  if (this != &r)
  {
    m_CTagsFileName = r.m_CTagsFileName;
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
  
wex::stc_data& wex::stc_data::CTagsFileName(const std::string& text)
{
  m_CTagsFileName = text;
  return *this;
}

wex::stc_data& wex::stc_data::Flags(
  stc_window_flags flags, data_action action)
{
  m_Data.Flags<stc_window_flags>(flags, m_WinFlags, action);

  return *this;
}
  
bool wex::stc_data::Inject() const
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
        m_STC->SetIndicator(indicator(0), 
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
        m_STC->FindNext(m_Data.Find(), m_Data.FindFlags());
      }
      else
      {
        m_STC->FindNext(m_Data.Find(), m_Data.FindFlags(), false);
      }
      return true;},
    [&]() {
      return m_STC->GetVi().Command(m_Data.Command().Command());});

  if (!m_Data.Window().Name().empty())
  {
    m_STC->SetName(m_Data.Window().Name());
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
  
wex::stc_data& wex::stc_data::Menu(
  stc_menu_flags flags, data_action action)
{
  m_Data.Flags<stc_menu_flags>(flags, m_MenuFlags, action);

  return *this;
}
