////////////////////////////////////////////////////////////////////////////////
// Name:      stream.cpp
// Purpose:   Implementation of wex::stream class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
#include <iostream>
#include <wex/stream.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/util.h>

bool wex::stream::m_Asked = false;

wex::stream::stream(const path& filename, const tool& tool)
  : m_Path(filename)
  , m_Tool(tool)
  , m_FRD(find_replace_data::Get())
  , m_Threshold(config(_("Max replacements")).get(-1))
{
}

bool wex::stream::Process(std::string& line, size_t line_no)
{
  bool match = false;
  int count = 1;
  int pos = -1;

  if (m_FRD->UseRegEx())
  {
    pos = m_FRD->RegExMatches(line);
    match = (pos >= 0);

    if (match && m_Tool.GetId() == ID_TOOL_REPLACE)
    {
      count = m_FRD->RegExReplaceAll(line);
      if (!m_Modified) m_Modified = (count > 0);
    }
  }
  else
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_FIND)
    {
      if (const auto it = (!m_FRD->MatchCase() ?
        std::search(line.begin(), line.end(), m_FindString.begin(), m_FindString.end(),
          [](char ch1, char ch2) {return std::toupper(ch1) == ch2;}):
#ifdef __WXGTK__
        std::search(line.begin(), line.end(), 
          std::boyer_moore_searcher(m_FindString.begin(), m_FindString.end())));
#else
        std::search(line.begin(), line.end(), 
          m_FindString.begin(), m_FindString.end()));
#endif
        it != line.end())
      {
        match = true;
        pos = it - line.begin();

        if (m_FRD->MatchWord() && 
            ((it != line.begin() && IsWordCharacter(*std::prev(it))) ||
              IsWordCharacter(*std::next(it, m_FindString.length()))))
        {
          match = false;
        }
      }
    }
    else
    {
      count = replace_all(
        line, 
        m_FRD->GetFindString(), 
        m_FRD->GetReplaceString(),
        &pos);

      match = (count > 0);
      if (!m_Modified) m_Modified = match;
    }
  }

  if (match)
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_FIND)
    {
      ProcessMatch(line, line_no, pos);
    }
    
    if (const auto ac = IncActionsCompleted(count);
      !m_Asked && m_Threshold != -1 && (ac - m_Prev > m_Threshold))
    {
      if (wxMessageBox(
        "More than " + std::to_string(m_Threshold) + " matches in: " + 
          m_Path.Path().string() + "?",
        _("Continue"),
        wxYES_NO | wxICON_QUESTION) == wxNO)
      {
        return false;
      }
      else
      {
        m_Asked = true;
      }
    }
  }

  return true;
}

bool wex::stream::ProcessBegin()
{
  if (
    !m_Tool.IsFindType() || 
    (m_Tool.GetId() == ID_TOOL_REPLACE && m_Path.GetStat().is_readonly()) ||
     find_replace_data::Get()->GetFindString().empty())
  {
    return false;
  }

  m_FindString = find_replace_data::Get()->GetFindString();
  m_Prev = m_Stats.Get(_("Actions Completed").ToStdString());
  m_Write = (m_Tool.GetId() == ID_TOOL_REPLACE);

  if (!find_replace_data::Get()->MatchCase())
  {
    for (auto & c : m_FindString) c = std::toupper(c);
  }
  
  return true;
}
  
bool wex::stream::RunTool()
{
  std::fstream fs(m_Path.Path(), std::ios_base::in);

  if (!fs.is_open() || !ProcessBegin())
  {
    return false;
  }

  m_Stats.m_Elements.Set(_("Files").ToStdString(), 1);
  
  int line_no = 0;
  std::string s;
  
  for (std::string line; std::getline(fs, line); )
  {
    if (!Process(line, line_no++)) return false;

    if (m_Write)
    {
      s += line + "\n";
    }
  }
  
  if (line_no <= 1 || (m_Write && s.empty()))
  {
    log_status(std::string("processing error"));
    return false;
  }
  else if (m_Modified && m_Write)
  {
    fs.close();
    fs.open(m_Path.Path(), std::ios_base::out);
    if (!fs.is_open()) return false;
    fs.write(s.c_str(), s.size());
  }

  ProcessEnd();

  return true;
}

void wex::stream::Reset()
{
  m_Asked = false;
}
