////////////////////////////////////////////////////////////////////////////////
// Name:      sort.cpp
// Purpose:   Implementation of wex::sort class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/sort.h>
#include <wex/factory/stc-undo.h>
#include <wex/factory/stc.h>

#include <map>
#include <set>
#include <vector>

namespace wex
{
namespace factory
{
class sort_data
{
public:
  template <typename T>
  const std::string
  to_string_separated(std::vector<std::string>& words, T ii) const
  {
    std::string text;

    for (auto& it : words)
    {
      text += it.replace(sort->m_pos, sort->m_len, *ii);

      if (&it != &words.back() || (&it == &words.back() && ends_with_separator))
      {
        text += separator;
      }

      ++ii;
    }

    return text;
  }

  template <typename T>
  const std::string
  to_string_cols_separated(const T& first, const T& last) const
  {
    std::string text;

    for (T it = first; it != last; ++it)
    {
      text += it->second;

      if (
        std::next(it) != last || (std::next(it) == last && ends_with_separator))
      {
        text += separator;
      }
    }

    return text;
  }

  const factory::sort* sort;
  const std::string    separator;
  const bool           ends_with_separator;
};

} // namespace factory
} // namespace wex

wex::factory::sort::sort(sort_t sort_t, size_t pos, size_t len)
  : m_sort_t(sort_t)
  , m_pos(pos)
  , m_len(len)
{
}

bool wex::factory::sort::selection(factory::stc* stc) const
{
  bool error = false;

  try
  {
    stc_undo undo(stc);
    return stc->SelectionIsRectangle() ? selection_block(stc) :
                                         selection_other(stc);
  }
  catch (std::exception& e)
  {
    log(e) << "sort::selection";
    error = true;
  }

  return !error;
}

bool wex::factory::sort::selection_block(factory::stc* stc) const
{
  const auto start_pos = stc->GetSelectionNStart(0);

  if (start_pos == -1)
  {
    log("sort::selection rectangle invalid start_pos") << start_pos;
    return false;
  }

  std::string selection;

  for (int i = 0; i < stc->GetSelections(); i++)
  {
    auto start = stc->GetSelectionNStart(i);
    auto end   = stc->GetSelectionNEnd(i);

    if (start == end)
    {
      log("sort::selection rectangle start equals end") << start;
      return false;
    }

    selection += stc->GetTextRange(start, end) + "\n";
  }

  const auto nr_cols =
    stc->GetColumn(stc->GetSelectionEnd()) - stc->GetColumn(start_pos);
  const auto  nr_lines = stc->LineFromPosition(stc->GetSelectionEnd()) -
                         stc->LineFromPosition(start_pos);
  const auto& text(sort(m_sort_t).string(selection, "\n"));

  boost::tokenizer<boost::char_separator<char>> tok(
    text,
    boost::char_separator<char>("\n"));

  auto it = tok.begin();

  for (int i = 0; i < stc->GetSelections() && it != tok.end(); i++)
  {
    auto start = stc->GetSelectionNStart(i);
    auto end   = stc->GetSelectionNEnd(i);
    stc->Replace(start, end, *it++);
  }

  stc->SelectNone();
  stc->SetCurrentPos(start_pos);

  for (int j = 0; j < nr_cols; j++)
  {
    stc->CharRightRectExtend();
  }
  for (int i = 0; i < nr_lines; i++)
  {
    stc->LineDownRectExtend();
  }

  return true;
}

bool wex::factory::sort::selection_other(factory::stc* stc) const
{
  const auto start_pos = stc->GetSelectionStart();

  if (
    start_pos == -1 || m_pos > (size_t)stc->GetSelectionEnd() ||
    m_pos == std::string::npos || stc->GetSelectionEmpty())
  {
    log("sort::selection") << start_pos << m_pos << stc->GetSelectionEnd();
    return false;
  }

  const auto& text(sort(m_sort_t).string(stc->get_selected_text(), stc->eol()));

  stc->ReplaceSelection(text);
  stc->SetSelection(start_pos, start_pos + text.size());

  return true;
}

const std::string wex::factory::sort::string(
  const std::string& input,
  const std::string& sep) const
{
  if (input.empty() || sep.empty())
  {
    return input;
  }

  std::map<std::string, std::string>      m;
  std::multimap<std::string, std::string> mm;
  std::multiset<std::string>              ms;
  std::vector<std::string>                words;

  boost::tokenizer<boost::char_separator<char>> tok(
    input,
    boost::char_separator<char>(sep.c_str()));

  for (auto it = tok.begin(); it != tok.end(); ++it)
  {
    const std::string& word(*it);

    // Use an empty key if line is too short.
    std::string key;

    if (m_pos < word.length())
    {
      key = word.substr(m_pos, m_len);
    }

    if (m_len == std::string::npos)
    {
      if (m_sort_t[SORT_UNIQUE])
      {
        m.insert({key, word});
      }
      else
      {
        mm.insert({key, word});
      }
    }
    else
    {
      words.emplace_back(word);
      ms.insert(key);
    }
  }

  const sort_data sd{this, sep, sep.contains(input.back())};

  if (m_len == std::string::npos)
  {
    if (m_sort_t[SORT_DESCENDING])
    {
      return (
        m_sort_t[SORT_UNIQUE] ?
          sd.to_string_cols_separated(m.rbegin(), m.rend()) :
          sd.to_string_cols_separated(mm.rbegin(), mm.rend()));
    }

    return (
      m_sort_t[SORT_UNIQUE] ?
        sd.to_string_cols_separated(m.begin(), m.end()) :
        sd.to_string_cols_separated(mm.begin(), mm.end()));
  }

  return (
    m_sort_t[SORT_DESCENDING] ? sd.to_string_separated(words, ms.rbegin()) :
                                sd.to_string_separated(words, ms.begin()));
}
