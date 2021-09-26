////////////////////////////////////////////////////////////////////////////////
// Name:      sort.cpp
// Purpose:   Implementation of wex::sort class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/sort.h>
#include <wex/factory/stc.h>

import<algorithm>;

wex::sort::sort(sort_t sort_t, size_t pos, size_t len)
  : m_sort_t(sort_t)
  , m_pos(pos)
  , m_len(len)
{
}

template <typename T> const std::string wex::sort::get_column(T first, T last)
{
  std::string text;

  for (T it = first; it != last; ++it)
  {
    text += it->second;
  }

  return text;
}

template <typename T>
const std::string wex::sort::get_lines(std::vector<std::string>& lines, T ii)
{
  std::string text;

  for (auto& it : lines)
  {
    text += it.replace(m_pos, m_len, *ii);
    ++ii;
  }

  return text;
}

bool wex::sort::selection(factory::stc* stc)
{
  bool error = false;

  try
  {
    return stc->SelectionIsRectangle() ? selection_block(stc) :
                                         selection_other(stc);
  }
  catch (std::exception& e)
  {
    log(e) << "sort::selection";
    error = true;
  }

  stc->EndUndoAction();

  return !error;
}

bool wex::sort::selection_block(factory::stc* stc)
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

  stc->BeginUndoAction();

  const auto nr_cols =
    stc->GetColumn(stc->GetSelectionEnd()) - stc->GetColumn(start_pos);
  const auto nr_lines = stc->LineFromPosition(stc->GetSelectionEnd()) -
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

bool wex::sort::selection_other(factory::stc* stc)
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

  stc->BeginUndoAction();
  stc->ReplaceSelection(text);
  stc->SetSelection(start_pos, start_pos + text.size());

  return true;
}

const std::string
wex::sort::string(const std::string& input, const std::string& sep)
{
  if (!std::all_of(sep.begin(), sep.end(), isspace))
  {
    log("sort::string separator should contain whitespace only") << sep;
    return input;
  }

  // Empty lines are not kept after sorting, as they are used as separator.
  std::map<std::string, std::string>      m;
  std::multimap<std::string, std::string> mm;
  std::multiset<std::string>              ms;
  std::vector<std::string>                lines;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         input,
         boost::char_separator<char>(sep.c_str())))
  {
    const std::string line = it + sep;

    // Use an empty key if line is to short.
    std::string key;

    if (m_pos < line.length())
    {
      key = line.substr(m_pos, m_len);
    }

    if (m_len == std::string::npos)
    {
      if (m_sort_t[SORT_UNIQUE])
        m.insert({key, line});
      else
        mm.insert({key, line});
    }
    else
    {
      lines.emplace_back(line);
      ms.insert(key);
    }
  }

  std::string text;

  if (m_len == std::string::npos)
  {
    if (m_sort_t[SORT_DESCENDING])
    {
      text =
        (m_sort_t[SORT_UNIQUE] ? get_column(m.rbegin(), m.rend()) :
                                 get_column(mm.rbegin(), mm.rend()));
    }
    else
    {
      text =
        (m_sort_t[SORT_UNIQUE] ? get_column(m.begin(), m.end()) :
                                 get_column(mm.begin(), mm.end()));
    }
  }
  else
  {
    text =
      (m_sort_t[SORT_DESCENDING] ? get_lines(lines, ms.rbegin()) :
                                   get_lines(lines, ms.begin()));
  }

  return text;
}
