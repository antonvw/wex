////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wex::find_replace_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/fdrepdlg.h>
#include <wex/frd.h>
#include <wex/config.h> 
#include <wex/ex-command.h>
#include <wex/util.h>
#include <easylogging++.h>

wex::find_replace_data* wex::find_replace_data::m_Self = nullptr;
std::string wex::find_replace_data::m_text_find = _("Find what").ToStdString();
std::string wex::find_replace_data::m_text_match_case = _("Match case").ToStdString();
std::string wex::find_replace_data::m_text_match_word = _("Match whole word").ToStdString();
std::string wex::find_replace_data::m_text_regex = _("Regular expression").ToStdString();
std::string wex::find_replace_data::m_text_replace_with = _("Replace with").ToStdString();
std::string wex::find_replace_data::m_text_search_down = _("Search down").ToStdString();

wex::find_replace_data::find_replace_data()
  : m_FindStrings(ex_command::type_t::FIND)
  , m_ReplaceStrings(ex_command::type_t::REPLACE)
  , m_frd(new wxFindReplaceData)
{
  int flags = 0;
  flags |= wxFR_DOWN *      (config(m_text_search_down).get(true));
  flags |= wxFR_MATCHCASE * (config(m_text_match_case).get(false));
  flags |= wxFR_WHOLEWORD * (config(m_text_match_word).get(false));

  m_frd->SetFlags(flags);

  // Start with this one, as it is used by set_find_string.
  set_use_regex(config(m_text_regex).get(m_use_regex));
  set_find_strings(config(m_text_find).get_list());
  set_replace_strings(config(m_text_replace_with).get_list());
}

wex::find_replace_data::~find_replace_data()
{
  config(m_text_match_case).set(match_case());
  config(m_text_match_word).set(match_word());
  config(m_text_regex).set(m_use_regex);
  config(m_text_search_down).set(search_down());
  
  delete m_frd;
}

wxFindReplaceData* wex::find_replace_data::data() 
{
  return m_frd;
}

wex::find_replace_data* wex::find_replace_data::get(bool createOnDemand)
{
  if (m_Self == nullptr && createOnDemand)
  {
    m_Self = new find_replace_data();
  }

  return m_Self;
}

const std::string wex::find_replace_data::get_find_string() const
{
  return m_frd->GetFindString().ToStdString();
}

const std::string wex::find_replace_data::get_replace_string() const 
{
  return m_frd->GetReplaceString().ToStdString();
}

bool wex::find_replace_data::match_case() const 
{
  return (m_frd->GetFlags() & wxFR_MATCHCASE) > 0;
}

bool wex::find_replace_data::match_word() const 
{
  return (m_frd->GetFlags() & wxFR_WHOLEWORD) > 0;
}

int wex::find_replace_data::regex_replace(std::string& text) const
{
  const auto words_begin = std::sregex_iterator(
    text.begin(), text.end(), m_regex);
  const auto words_end = std::sregex_iterator();  
  const int result = std::distance(words_begin, words_end);
  
  text = std::regex_replace(
    text, 
    m_regex, 
    get_replace_string(),
    std::regex_constants::format_default);

  return result;
}
  
int wex::find_replace_data::regex_search(const std::string& text) const
{
  if (std::smatch m; !std::regex_search(
    text, 
    m,
    m_regex))
    return -1;
  else
    return m.position();
}
  
bool wex::find_replace_data::search_down() const 
{
  return (m_frd->GetFlags() & wxFR_DOWN) > 0;
}

wex::find_replace_data* wex::find_replace_data::set(find_replace_data* frd)
{
  find_replace_data* old = m_Self;
  m_Self = frd;
  return old;
}

void wex::find_replace_data::set_find_string(const std::string& value)
{
  if (!m_FindStrings.set(value)) return;
  m_frd->SetFindString(value);
  set_use_regex(m_use_regex);
}

void wex::find_replace_data::set_find_strings(
  const std::list < std::string > & values)
{
  m_FindStrings.set(values);
  m_frd->SetFindString(m_FindStrings.get());
  set_use_regex(m_use_regex);
}

void wex::find_replace_data::set_match_case(bool value)
{
  auto flags = m_frd->GetFlags();
  
  if (value) flags |= wxFR_MATCHCASE;
  else       flags &= ~wxFR_MATCHCASE;
  
  m_frd->SetFlags(flags);
}

void wex::find_replace_data::set_match_word(bool value)
{
  auto flags = m_frd->GetFlags();
  
  if (value) flags |= wxFR_WHOLEWORD;
  else       flags &= ~wxFR_WHOLEWORD;
  
  m_frd->SetFlags(flags);
  
  // Match word and regular expression do not work together.
  if (value)
  {
    set_use_regex(false);
  }
}

void wex::find_replace_data::set_replace_string(const std::string& value)
{
  // value is allowed to be empty
  m_ReplaceStrings.set(value);
  m_frd->SetReplaceString(value);
}

void wex::find_replace_data::set_replace_strings(
  const std::list < std::string > & value)
{
  m_ReplaceStrings.set(value);
  m_frd->SetReplaceString(m_ReplaceStrings.get());
}

void wex::find_replace_data::set_search_down(bool value)
{
  auto flags = m_frd->GetFlags();
  
  if (value) flags |= wxFR_DOWN;
  else       flags &= ~wxFR_DOWN;
  
  m_frd->SetFlags(flags);
}

void wex::find_replace_data::set_use_regex(bool value) 
{
  if (!value)
  {
    m_use_regex = false;
    return;
  }
  
  try 
  {
    std::regex::flag_type flags = std::regex::ECMAScript;
    if (!match_case()) flags |= std::regex::icase;
  
    m_regex = std::regex(get_find_string(), flags);
    m_use_regex = true;

    VLOG(9) << "frd regex find: " << 
      get_find_string() << " replace: " << get_replace_string();
  }
  catch (std::regex_error& e) 
  {
    m_use_regex = false;
    std::stringstream ss;
    ss << "regex error: " << e.what() << " " << get_find_string();
    log_status(ss.str());
  }
}
