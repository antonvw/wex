////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.cpp
// Purpose:   Implementation of wex::textctrl_input class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/textctrl.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <easylogging++.h>

wex::textctrl_input::textctrl_input(ex_command::type_t type) 
  : m_Type(type)
  , m_Name([](ex_command::type_t type) {
      switch (type)
      {
        case ex_command::type_t::CALC: return std::string("excalc");
        case ex_command::type_t::COMMAND: return std::string("excommand");
        case ex_command::type_t::EXEC: return std::string("exexec");
        case ex_command::type_t::FIND: return find_replace_data::text_find();
        case ex_command::type_t::FIND_MARGIN: return std::string("exmargin");
        case ex_command::type_t::REPLACE: return find_replace_data::text_replace_with();
        default: return std::string("exother");
      }}(type))
  , m_Values(config(m_Name).get_list())
  , m_Iterator(m_Values.cbegin())
{
  VLOG(9) << "TCI " << m_Name << " size: " << m_Values.size();
}

wex::textctrl_input::~textctrl_input() 
{
  wex::config(m_Name).set(m_Values);
}

const std::string wex::textctrl_input::get() const 
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
  
bool wex::textctrl_input::set(const std::string& value)
{
  if (value.empty()) return false;

  m_Values.remove(value);
  m_Values.push_front(value);
  m_Iterator = m_Values.cbegin();
  config(m_Name).set(m_Values);
  
  return true;
}

bool wex::textctrl_input::set(int key, wxTextCtrl* tc) 
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
    tc->SetValue(get());
    tc->SetInsertionPointEnd();
  }
  
  return true;
}

void wex::textctrl_input::set(const std::list < std::string > & values)
{
  m_Values.assign(values.cbegin(), values.cend());
  m_Iterator = m_Values.cbegin();
  config(m_Name).set(values);
}
