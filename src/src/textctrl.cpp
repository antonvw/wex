////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.cpp
// Purpose:   Implementation of wex::textctrl_input class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/textctrl.h>
#include <wex/textctrl.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/log.h>

wex::textctrl_input::textctrl_input(ex_command::type_t type) 
  : m_type(type)
  , m_name([](ex_command::type_t type) {
      switch (type)
      {
        case ex_command::type_t::CALC: 
          return std::string("ex-cmd.calc");

        case ex_command::type_t::COMMAND: 
          return std::string("ex-cmd.command");

        case ex_command::type_t::EXEC: 
          return std::string("ex-cmd.exec");

        case ex_command::type_t::FIND: 
          return find_replace_data::text_find();

        case ex_command::type_t::FIND_MARGIN: 
          return std::string("ex-cmd.margin");

        case ex_command::type_t::REPLACE: 
          return find_replace_data::text_replace_with();

        default: return std::string("ex-cmd.other");
      }}(type))
  , m_values(config(m_name).get(std::list<std::string>{}))
  , m_iterator(m_values.cbegin())
{
  log::verbose("TCI") << m_name << "size:" << m_values.size();
}

wex::textctrl_input::~textctrl_input() 
{
  wex::config(m_name).set(m_values);
}

const std::string wex::textctrl_input::get() const 
{
  try
  {
    return m_iterator != m_values.end() ? *m_iterator: std::string();
  }
  catch (std::exception& e)
  {
    log(e) << "TCI:" << m_name;
    return std::string();
  }
}
  
void wex::textctrl_input::set(const std::string& value)
{
  assert(!value.empty());

  m_values.remove(value);
  m_values.push_front(value);
  m_iterator = m_values.cbegin();
  config(m_name).set(m_values);
}

bool wex::textctrl_input::set(int key, wxTextCtrl* tc) 
{
  if (m_values.empty()) return false;
  
  const int page = 10;
  
  switch (key)
  {
    case WXK_DOWN: if (m_iterator != m_values.cbegin()) m_iterator--; break;
    case WXK_END: m_iterator = m_values.cend(); m_iterator--; break;
    case WXK_HOME: m_iterator = m_values.cbegin(); break;
    case WXK_UP: if (m_iterator != m_values.cend()) m_iterator++; break;
    
    case WXK_PAGEDOWN: 
      if (std::distance(m_values.cbegin(), m_iterator) > page)
        std::advance(m_iterator, -page); 
      else
        m_iterator = m_values.cbegin();
      break;
    case WXK_PAGEUP: 
      if (std::distance(m_iterator, m_values.cend()) > page)
        std::advance(m_iterator, page); 
      else
      {
        m_iterator = m_values.cend();
        m_iterator--;
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

void wex::textctrl_input::set(const wxTextCtrl* tc) 
{
  if (const auto v(tc->GetValue().ToStdString()); !v.empty())
  {
    set(v);
  }
}
  
void wex::textctrl_input::set(const std::list < std::string > & values)
{
  m_values.assign(values.cbegin(), values.cend());
  m_iterator = m_values.cbegin();
  config(m_name).set(values);
}
