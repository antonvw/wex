////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wex::find_replace_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/data/find.h>
#include <wex/factory/ex-command.h>
#include <wex/ui/frd.h>
#include <wx/fdrepdlg.h>

wex::find_replace_data::find_replace_data()
  : m_find_strings(ex_command::type_t::FIND, find_replace_data::text_find())
  , m_replace_strings(
      ex_command::type_t::REPLACE,
      find_replace_data::text_replace_with())
{
  set_find_strings(config(text_find()).get(ex_commandline_input::values_t{}));
  set_replace_strings(
    config(text_replace_with()).get(ex_commandline_input::values_t{}));
}

wex::find_replace_data* wex::find_replace_data::get(bool createOnDemand)
{
  if (m_self == nullptr && createOnDemand)
  {
    m_self = new find_replace_data();
  }

  return m_self;
}

bool wex::find_replace_data::match(const std::string& text, const data::find& f)
  const
{
  if (text.empty() || f.text().empty())
  {
    return false;
  }

  if (!get()->match_word())
  {
    if (!get()->match_case())
    {
      if (boost::algorithm::to_upper_copy(text).contains(
            boost::algorithm::to_upper_copy(f.text())))
      {
        return true;
      }
    }
    else
    {
      if (text.contains(f.text()))
      {
        return true;
      }
    }
  }
  else
  {
    if (!get()->match_case())
    {
      if (boost::algorithm::iequals(f.text(), text))
      {
        return true;
      }
    }
    else
    {
      if (f.text() == text)
      {
        return true;
      }
    }
  }

  return false;
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
  const ex_commandline_input::values_t& values)
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
  const ex_commandline_input::values_t& value)
{
  m_replace_strings.set(value);
  data()->SetReplaceString(m_replace_strings.get());

  log::trace("frd set_replace_strings") << get_replace_string();
}
