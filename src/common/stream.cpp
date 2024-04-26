////////////////////////////////////////////////////////////////////////////////
// Name:      stream.cpp
// Purpose:   Implementation of wex::stream class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/common/stream.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/beautify.h>
#include <wex/factory/frd.h>
#include <wx/msgdlg.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>

wex::stream::stream(
  factory::find_replace_data* frd,
  const wex::path&            filename,
  const tool&                 tool,
  wxEvtHandler*               eh)
  : m_path(filename)
  , m_tool(tool)
  , m_frd(frd)
  , m_threshold(config(_("fif.Max replacements")).get(-1))
  , m_eh(eh)
{
}

bool wex::stream::process(std::string& text, size_t line_no)
{
  bool match = false;
  int  count = 1;
  int  pos   = -1;

  if (m_frd->is_regex())
  {
    pos   = m_frd->regex_search(text);
    match = (pos >= 0);

    if (match && (m_tool.id() == ID_TOOL_REPLACE))
    {
      count = m_frd->regex_replace(text);
      if (!m_modified)
      {
        m_modified = (count > 0);
      }
    }
  }
  else
  {
    if (m_tool.id() == ID_TOOL_REPORT_FIND)
    {
      auto text_find(text);

      if (!m_frd->match_case())
      {
        boost::algorithm::to_upper(text_find);
      }

      if (const auto it = std::search(
            text_find.begin(),
            text_find.end(),
            std::boyer_moore_searcher(
              m_find_string.begin(),
              m_find_string.end()));
          it != text_find.end())
      {
        match = true;
        pos   = it - text_find.begin();

        if (
          m_frd->match_word() &&
          ((it != text_find.begin() && is_word_character(*std::prev(it))) ||
           is_word_character(*std::next(it, m_find_string.length()))))
        {
          match = false;
        }
      }
    }
    else
    {
      count = replace_all(text, &pos);

      match = (count > 0);
      if (!m_modified)
      {
        m_modified = match;
      }
    }
  }

  if (match)
  {
    if (m_tool.id() == ID_TOOL_REPORT_FIND && m_eh != nullptr)
    {
      process_match(path_match(path(), text, line_no, pos));
    }

    const auto ac = m_stats.inc_actions_completed(count);

    if (!m_asked && m_threshold != -1 && (ac - m_prev > m_threshold))
    {
      if (
        wxMessageBox(
          "More than " + std::to_string(m_threshold) +
            " matches in: " + m_path.string() + "?",
          _("Continue"),
          wxYES_NO | wxICON_QUESTION) == wxNO)
      {
        return false;
      }

      m_asked = true;
    }
  }

  return true;
}

bool wex::stream::process_begin()
{
  if (
    m_frd == nullptr || m_frd->get_find_string().empty() ||
    !m_tool.is_find_type() ||
    (m_tool.id() == ID_TOOL_REPLACE && m_path.stat().is_readonly()))
  {
    log("stream::process_begin") << m_path;
    return false;
  }

  m_prev  = m_stats.get(_("Actions Completed").ToStdString());
  m_write = (m_tool.id() == ID_TOOL_REPLACE);

  if (!m_frd->is_regex())
  {
    m_find_string = m_frd->get_find_string();

    if (!m_frd->match_case())
    {
      boost::algorithm::to_upper(m_find_string);
    }
  }

  return true;
}

void wex::stream::process_match(const path_match& m)
{
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_LIST_MATCH);
  event.SetClientData(new path_match(m));
  wxPostEvent(m_eh, event);
}

int wex::stream::replace_all(std::string& text, int* match_pos)
{
  int         count  = 0;
  bool        update = false;
  const auto& search(m_frd->get_find_string());
  const auto& replace(m_frd->get_replace_string());

  for (size_t pos = 0; (pos = text.find(search, pos)) != std::string::npos;)
  {
    if (!update)
    {
      *match_pos = static_cast<int>(pos);
      update     = true;
    }

    text.replace(pos, search.length(), replace);
    pos += replace.length();

    count++;
  }

  return count;
}

bool wex::stream::run_tool()
{
  std::fstream fs(m_path.data(), std::ios_base::in);
  if (!fs.is_open())
  {
    log("stream::open") << m_path;
    return false;
  }

  if (!process_begin())
  {
    return false;
  }

  m_asked = false;
  std::string s;
  int         line_no = 0;

  for (std::string line; std::getline(fs, line);)
  {
    if (!process(line, line_no++))
    {
      return false;
    }

    if (m_write)
    {
      s += line + "\n";
    }
  }

  if (m_write && s.empty())
  {
    log("stream processing error") << m_path;
    return false;
  }

  if (m_modified && m_write)
  {
    fs.close();
    fs.open(m_path.data(), std::ios_base::out);
    if (!fs.is_open())
    {
      return false;
    }

    fs.write(s.c_str(), s.size());

    if (factory::beautify b;
        b.is_active() && b.is_auto() && b.is_supported(m_path))
    {
      fs.close();
      b.file(m_path);
    }
  }

  return true;
}
