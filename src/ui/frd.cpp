////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wex::find_replace_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/ex-command.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wx/fdrepdlg.h>

wex::find_replace_data::find_replace_data()
  : m_find_strings(ex_command::type_t::FIND)
  , m_replace_strings(ex_command::type_t::REPLACE)
{
  set_find_strings(config(text_find()).get(std::list<std::string>{}));
  set_replace_strings(
    config(text_replace_with()).get(std::list<std::string>{}));
}

wex::find_replace_data* wex::find_replace_data::get(bool createOnDemand)
{
  if (m_self == nullptr && createOnDemand)
  {
    m_self = new find_replace_data();
  }

  return m_self;
}

wex::find_replace_data* wex::find_replace_data::set(find_replace_data* frd)
{
  find_replace_data* old = m_self;
  m_self                 = frd;
  return old;
}

void wex::find_replace_data::set_find_string(const std::string& value)
{
  m_find_strings.set(value);

  factory::find_replace_data::set_find_string(value);
}

void wex::find_replace_data::set_find_strings(
  const std::list<std::string>& values)
{
  m_find_strings.set(values);
  data()->SetFindString(m_find_strings.get());
  set_regex(is_regex());

  log::trace("frd set_find_strings") << get_find_string();
}

void wex::find_replace_data::set_replace_string(const std::string& value)
{
  // value is allowed to be empty
  m_replace_strings.set(value);

  factory::find_replace_data::set_replace_string(value);
}

void wex::find_replace_data::set_replace_strings(
  const std::list<std::string>& value)
{
  m_replace_strings.set(value);
  data()->SetReplaceString(m_replace_strings.get());

  log::trace("frd set_replace_strings") << get_replace_string();
}
