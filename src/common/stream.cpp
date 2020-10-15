////////////////////////////////////////////////////////////////////////////////
// Name:      stream.cpp
// Purpose:   Implementation of wex::stream class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <cctype>
#ifndef __WXMSW__
#include <algorithm>
#include <functional>
#endif
#include <fstream>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/stream.h>

wex::stream::stream(const path& filename, const tool& tool)
  : m_path(filename)
  , m_tool(tool)
  , m_frd(find_replace_data::get())
  , m_threshold(config(_("fif.Max replacements")).get(-1))
{
}

bool wex::stream::process(std::string& text, size_t line_no)
{
  bool match = false;
  int  count = 1;
  int  pos   = -1;

  if (m_frd->use_regex())
  {
    pos   = m_frd->regex_search(text);
    match = (pos >= 0);

    if (match && (m_tool.id() == ID_TOOL_REPLACE))
    {
      count = m_frd->regex_replace(text);
      if (!m_modified)
        m_modified = (count > 0);
    }
  }
  else
  {
    if (m_tool.id() == ID_TOOL_REPORT_FIND)
    {
      if (const auto it =
            (!m_frd->match_case() ? std::search(
                                      text.begin(),
                                      text.end(),
                                      m_find_string.begin(),
                                      m_find_string.end(),
                                      [](char ch1, char ch2) {
                                        return std::toupper(ch1) == ch2;
                                      }) :
#ifdef __WXGTK__
                                    std::search(
                                      text.begin(),
                                      text.end(),
                                      std::boyer_moore_searcher(
                                        m_find_string.begin(),
                                        m_find_string.end())));
#else
                                    std::search(
                                      text.begin(),
                                      text.end(),
                                      m_find_string.begin(),
                                      m_find_string.end()));
#endif
          it != text.end())
      {
        match = true;
        pos   = it - text.begin();

        if (
          m_frd->match_word() &&
          ((it != text.begin() && is_word_character(*std::prev(it))) ||
           is_word_character(*std::next(it, m_find_string.length()))))
        {
          match = false;
        }
      }
    }
    else
    {
      count = replace_all(
        text,
        m_frd->get_find_string(),
        m_frd->get_replace_string(),
        &pos);

      match = (count > 0);
      if (!m_modified)
        m_modified = match;
    }
  }

  if (match)
  {
    if (m_tool.id() == ID_TOOL_REPORT_FIND)
    {
      process_match(text, line_no, pos);
    }

    if (const auto ac = inc_actions_completed(count);
        !m_asked && m_threshold != -1 && (ac - m_prev > m_threshold))
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
      else
      {
        m_asked = true;
      }
    }
  }

  return true;
}

bool wex::stream::process_begin()
{
  if (
    m_frd->get_find_string().empty() || !m_tool.is_find_type() ||
    (m_tool.id() == ID_TOOL_REPLACE && m_path.stat().is_readonly()))
  {
    return false;
  }

  m_prev  = m_stats.get(_("Actions Completed").ToStdString());
  m_write = (m_tool.id() == ID_TOOL_REPLACE);

  if (!m_frd->use_regex())
  {
    m_find_string = m_frd->get_find_string();

    if (!m_frd->match_case())
    {
      for (auto& c : m_find_string)
        c = std::toupper(c);
    }
  }

  return true;
}

bool wex::stream::run_tool()
{
  if (std::fstream fs(m_path.data(), std::ios_base::in); !fs.is_open())
  {
    log("open") << m_path;
    return false;
  }
  else if (!process_begin())
  {
    log("begin") << m_path;
    return false;
  }
  else
  {
    m_stats.m_elements.set(_("Files").ToStdString(), 1);

    int         line_no = 0;
    std::string s;

    log::verbose("run_tool") << m_path;

    for (std::string line; std::getline(fs, line);)
    {
      if (!process(line, line_no++))
        return false;
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
    else if (m_modified && m_write)
    {
      fs.close();
      fs.open(m_path.data(), std::ios_base::out);
      if (!fs.is_open())
        return false;
      fs.write(s.c_str(), s.size());
    }

    process_end();
    return true;
  }
}

void wex::stream::reset()
{
  m_asked = false;
}
