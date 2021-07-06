////////////////////////////////////////////////////////////////////////////////
// Name:      frd.cpp
// Purpose:   Implementation of wex::factory::find_replace_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/factory/frd.h>
#include <wex/log.h>
#include <wx/fdrepdlg.h>
#include <wx/translation.h>

wex::factory::find_replace_data::find_replace_data()
  : m_frd(new wxFindReplaceData)
{
  if (m_text_find.empty())
  {
    m_text_find         = _("fif.Find what");
    m_text_match_case   = _("fif.Match case");
    m_text_match_word   = _("fif.Match whole word");
    m_text_regex        = _("fif.Regular expression");
    m_text_replace_with = _("fif.Replace with");
    m_text_search_down  = _("fif.Search down");
  }

  int flags = 0;
  flags |= wxFR_DOWN * (config(m_text_search_down).get(true));
  flags |= wxFR_MATCHCASE * (config(m_text_match_case).get(false));
  flags |= wxFR_WHOLEWORD * (config(m_text_match_word).get(false));

  m_frd->SetFlags(flags);

  // Start with this one, as it is used by set_find_string.
  set_regex(config(m_text_regex).get(m_use_regex));
}

wex::factory::find_replace_data::~find_replace_data()
{
  delete m_frd;
}

wxFindReplaceData* wex::factory::find_replace_data::data()
{
  return m_frd;
}

const std::string wex::factory::find_replace_data::get_find_string() const
{
  return m_frd->GetFindString();
}

const std::string wex::factory::find_replace_data::get_replace_string() const
{
  return m_frd->GetReplaceString();
}

bool wex::factory::find_replace_data::match_case() const
{
  return (m_frd->GetFlags() & wxFR_MATCHCASE) > 0;
}

bool wex::factory::find_replace_data::match_word() const
{
  return (m_frd->GetFlags() & wxFR_WHOLEWORD) > 0;
}

int wex::factory::find_replace_data::regex_replace(std::string& text) const
{
  const auto words_begin =
    std::sregex_iterator(text.begin(), text.end(), m_regex);
  const auto words_end = std::sregex_iterator();
  const int  result    = std::distance(words_begin, words_end);

  text = std::regex_replace(
    text,
    m_regex,
    get_replace_string(),
    // Otherwise \2 \1 in replacement does not work,
    // though that actually is ECMAScript??
    std::regex_constants::format_sed);

  return result;
}

int wex::factory::find_replace_data::regex_search(const std::string& text) const
{
  if (std::smatch m; !std::regex_search(text, m, m_regex))
    return -1;
  else
    return m.position();
}

bool wex::factory::find_replace_data::search_down() const
{
  return (m_frd->GetFlags() & wxFR_DOWN) > 0;
}

void wex::factory::find_replace_data::set_find_string(const std::string& value)
{
  m_frd->SetFindString(value);
  set_regex(m_use_regex);

  log::trace("frd set_find_string") << get_find_string();
}

void wex::factory::find_replace_data::set_match_case(bool value)
{
  auto flags = m_frd->GetFlags();

  if (value)
    flags |= wxFR_MATCHCASE;
  else
    flags &= ~wxFR_MATCHCASE;

  m_frd->SetFlags(flags);

  config(m_text_match_case).set(match_case());
}

void wex::factory::find_replace_data::set_match_word(bool value)
{
  auto flags = m_frd->GetFlags();

  if (value)
    flags |= wxFR_WHOLEWORD;
  else
    flags &= ~wxFR_WHOLEWORD;

  m_frd->SetFlags(flags);

  config(m_text_match_word).set(match_word());
}

void wex::factory::find_replace_data::set_regex(bool value)
{
  if (!value)
  {
    m_use_regex = false;
    config(m_text_regex).set(m_use_regex);
    return;
  }

  try
  {
    std::regex::flag_type flags = std::regex::ECMAScript;

    if (!match_case())
    {
      flags |= std::regex::icase;
    }

    m_regex     = std::regex(get_find_string(), flags);
    m_use_regex = true;

    log::trace("frd set_regex") << get_find_string();
  }
  catch (std::regex_error& e)
  {
    m_use_regex = false;
    log::status(e.what()) << "regex" << get_find_string();
  }

  config(m_text_regex).set(m_use_regex);
}

void wex::factory::find_replace_data::set_replace_string(
  const std::string& value)
{
  // value is allowed to be empty
  m_frd->SetReplaceString(value);

  log::trace("frd set_replace_string") << get_replace_string();
}

void wex::factory::find_replace_data::set_search_down(bool value)
{
  auto flags = m_frd->GetFlags();

  if (value)
    flags |= wxFR_DOWN;
  else
    flags &= ~wxFR_DOWN;

  m_frd->SetFlags(flags);

  config(m_text_search_down).set(search_down());
}
