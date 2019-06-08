////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.cpp
// Purpose:   Implementation of wex::stc_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc-data.h>
#include <wex/indicator.h>
#include <wex/stc.h>

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
  : m_Data(control_data().window(data))
  , m_STC(stc)
{
}

wex::stc_data& wex::stc_data::operator=(const stc_data& r)
{
  if (this != &r)
  {
    m_indicator_no = r.m_indicator_no;
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
  
wex::stc_data& wex::stc_data::flags(
  window_t flags, control_data::action_t action)
{
  m_Data.flags<flags.size()>(flags, m_WinFlags, action);

  return *this;
}
  
wex::stc_data& wex::stc_data::indicator_no(indicator_t t)
{
  m_indicator_no = t;

  return *this;
}
  
bool wex::stc_data::inject() const
{
  if (m_STC == nullptr) return false;
  
  bool injected = m_Data.inject(
    [&]() {
      if (m_Data.line() > 0)
      {
        m_STC->GotoLine(m_Data.line() -1);
        m_STC->EnsureVisible(m_Data.line() -1);
        m_STC->EnsureCaretVisible();
        m_STC->SetIndicatorCurrent(m_indicator_no);
        m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
        m_STC->set_indicator(indicator(m_indicator_no), 
          m_STC->PositionFromLine(m_Data.line() -1), 
          m_Data.col() > 0 ? 
            m_STC->PositionFromLine(m_Data.line() -1) + m_Data.col() - 1:
            m_STC->GetLineEndPosition(m_Data.line() -1));
      }
      else if (m_Data.line() == DATA_NUMBER_NOT_SET)
      {
        return false;
      }
      else
      {
        m_STC->DocumentEnd();
      }
      return true;},
    [&]() {
      const int max = (m_Data.line() > 0) ? 
        m_STC->GetLineEndPosition(m_Data.line() - 1): 0;
      const int asked = m_STC->GetCurrentPos() + m_Data.col() - 1;
          
      m_STC->SetCurrentPos(asked < max ? asked: max);
          
      // Reset selection, seems necessary.
      m_STC->SelectNone();
      return true;},
    [&]() {
      if (m_Data.line() > 0)
      {
        const int start_pos = m_STC->PositionFromLine(m_Data.line() -1);
        const int end_pos = m_STC->GetLineEndPosition(m_Data.line() -1);

        m_STC->set_search_flags(-1);
        m_STC->SetTargetStart(start_pos);
        m_STC->SetTargetEnd(end_pos);

        if (m_STC->SearchInTarget(m_Data.find()) != -1)
        {
          m_STC->SetSelection(m_STC->GetTargetStart(), m_STC->GetTargetEnd());
        }
      }
      else if (m_Data.line() == DATA_NUMBER_NOT_SET)
      {
        m_STC->find_next(m_Data.find(), m_Data.find_flags());
      }
      else
      {
        m_STC->find_next(m_Data.find(), m_Data.find_flags(), false);
      }
      return true;},
    [&]() {
      return m_STC->get_vi().command(m_Data.command().command());});

  if (!m_Data.window().name().empty())
  {
    m_STC->SetName(m_Data.window().name());
  }
  
  if ( m_WinFlags[WIN_READ_ONLY] ||
      (m_STC->get_filename().file_exists() && m_STC->get_filename().is_readonly()))
  {
    m_STC->SetReadOnly(true);
    injected = true;
  }

  if (m_STC->get_hexmode().set(m_WinFlags[WIN_HEX]))
  {
    injected = true;
  }
  
  if (injected)
  {
    m_STC->properties_message();
  }
  
  return injected;
}
  
wex::stc_data& wex::stc_data::menu(
  menu_t flags, control_data::action_t action)
{
  m_Data.flags<flags.size()>(flags, m_MenuFlags, action);

  return *this;
}
