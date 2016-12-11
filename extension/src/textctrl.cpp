////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.cpp
// Purpose:   Implementation of wxExTextCtrlInput class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/textctrl.h>
#include <wx/extension/util.h>

wxExTextCtrlInput::wxExTextCtrlInput(const std::string& name) 
  : m_Name(name)
  , m_Values(wxExListFromConfig(name))
  , m_Iterator(m_Values.cbegin()) 
{
}

wxExTextCtrlInput::~wxExTextCtrlInput() 
{
  wxExListToConfig(m_Values, m_Name);
}

const std::string wxExTextCtrlInput::Get() const 
{
  return m_Iterator != m_Values.end() ? *m_Iterator: std::string();
}
  
bool wxExTextCtrlInput::Set(const std::string& value)
{
  if (value.empty()) return false;

  m_Values.remove(value);
  m_Values.push_front(value);
  m_Iterator = m_Values.cbegin();
  wxExListToConfig(m_Values, m_Name);
  
  return true;
}

bool wxExTextCtrlInput::Set(int key, wxTextCtrl* tc) 
{
  if (m_Values.empty()) return false;
  
  const int page = 10;
  
  switch (key)
  {
    case WXK_DOWN: if (m_Iterator != m_Values.cbegin()) m_Iterator--; break;
    case WXK_END: m_Iterator = m_Values.cend(); m_Iterator--; break;
    case WXK_HOME: m_Iterator = m_Values.cbegin(); break;
    case WXK_UP: if (m_Iterator != m_Values.cend()) m_Iterator++; break;
    
    case WXK_PAGEDOWN: 
      if (std::distance(m_Values.cbegin(), m_Iterator) > page)
        std::advance(m_Iterator, -page); 
      else
        m_Iterator = m_Values.cbegin();
      break;
    case WXK_PAGEUP: 
      if (std::distance(m_Iterator, m_Values.cend()) > page)
        std::advance(m_Iterator, page); 
      else
      {
        m_Iterator = m_Values.cend();
        m_Iterator--;
      }
      break;
    
    default: return false;
  }

  if (tc != nullptr)
  {
    tc->SetValue(Get());
    tc->SetInsertionPointEnd();
  }
  
  return true;
}

void wxExTextCtrlInput::Set(const std::list < std::string > & values)
{
  m_Values.assign(values.cbegin(), values.cend());
  m_Iterator = m_Values.cbegin();
  wxExListToConfig(values, m_Name);
}
