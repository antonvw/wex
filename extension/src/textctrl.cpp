////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.cpp
// Purpose:   Implementation of wex::textctrl_input class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/textctrl.h>
#include <wx/extension/ex-command.h>
#include <wx/extension/frd.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>
#include <easylogging++.h>

wex::textctrl_input::textctrl_input(ex_command_type type) 
  : m_Type(type)
  , m_Name([](ex_command_type type) {
      switch (type)
      {
        case ex_command_type::CALC: return std::string("excalc");
        case ex_command_type::COMMAND: return std::string("excommand");
        case ex_command_type::EXEC: return std::string("exexec");
        case ex_command_type::FIND: return find_replace_data::GetTextFindWhat();
        case ex_command_type::FIND_MARGIN: return std::string("exmargin");
        case ex_command_type::REPLACE: return find_replace_data::GetTextReplaceWith();
        default: return std::string("exother");
      }}(type))
  , m_Values(list_from_config(m_Name))
  , m_Iterator(m_Values.cbegin())
{
  VLOG(9) << "TCI " << m_Name << " size: " << m_Values.size();
}

wex::textctrl_input::~textctrl_input() 
{
  list_to_config(m_Values, m_Name);
}

const std::string wex::textctrl_input::Get() const 
{
  try
  {
    return m_Iterator != m_Values.end() ? *m_Iterator: std::string();
  }
  catch (std::exception& e)
  {
    log(e) << "TCI:" << m_Name;
    return std::string();
  }
}
  
bool wex::textctrl_input::Set(const std::string& value)
{
  if (value.empty()) return false;

  m_Values.remove(value);
  m_Values.push_front(value);
  m_Iterator = m_Values.cbegin();
  list_to_config(m_Values, m_Name);
  
  return true;
}

bool wex::textctrl_input::Set(int key, wxTextCtrl* tc) 
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

void wex::textctrl_input::Set(const std::list < std::string > & values)
{
  m_Values.assign(values.cbegin(), values.cend());
  m_Iterator = m_Values.cbegin();
  list_to_config(values, m_Name);
}
