////////////////////////////////////////////////////////////////////////////////
// Name:      stream.cpp
// Purpose:   Implementation of wxExStream class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <fstream>
#include <iostream>
#include <wx/extension/stream.h>
#include <wx/extension/frd.h>
#include <wx/extension/util.h>

wxExStream::wxExStream(const wxExFileName& filename, const wxExTool& tool)
  : m_FileName(filename)
  , m_Tool(tool)
  , m_FRD(wxExFindReplaceData::Get())
{
}

bool wxExStream::Process(std::string& line, size_t line_no)
{
  bool modified = false;
  bool match = false;
  int count = 1;

  if (!m_FRD->UseRegEx())
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_FIND)
    {
      if (!m_FRD->MatchCase())
      {
        std::transform(line.begin(), line.end(), line.begin(), toupper);
      }

      const size_t start = line.find(m_FindString);

      if (start != std::string::npos)
      {
        match = true;

        if (m_FRD->MatchWord())
        {
          if ((start > 0 && IsWordCharacter(line[start - 1])) ||
               IsWordCharacter(line[start + m_FindString.length()]))
          {
            match = false;
          }
        }
      }
    }
    else
    {
      count = wxExReplaceAll(line, m_FRD->GetFindString(), m_FRD->GetReplaceString());

      if (count > 0)
      {
        match = true;
        modified = true;
      }
    }
  }
  else
  {
    match = m_FRD->RegExMatches(line);

    if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
    {
      count = m_FRD->RegExReplaceAll(line);
      if (count > 0) modified = true;
    }
  }

  if (match)
  {
    IncActionsCompleted(count);
    ProcessMatch(line, line_no);
    
    if (m_Stats.Get(_("Actions Completed").ToStdString()) - m_Prev > 250)
    {
      wxLogMessage("too many matches, reconsider your search");
      return false;
    }
  }

  return true;
}

bool wxExStream::ProcessBegin()
{
  if (
    !m_Tool.IsFindType() || 
    (m_Tool.GetId() == ID_TOOL_REPORT_REPLACE && m_FileName.GetStat().IsReadOnly()) ||
     wxExFindReplaceData::Get()->GetFindString().empty())
  {
    return false;
  }

  m_Write = (m_Tool.GetId() == ID_TOOL_REPORT_REPLACE);
  m_FindString = wxExFindReplaceData::Get()->GetFindString();
  m_Prev = m_Stats.Get(_("Actions Completed").ToStdString());

  if (!wxExFindReplaceData::Get()->MatchCase())
  {
    std::transform(
      m_FindString.begin(), m_FindString.end(), m_FindString.begin(), toupper);
  }
  
  return true;
}
  
bool wxExStream::RunTool()
{
  std::ifstream ifs(m_FileName.GetFullPath().c_str());

  if (!ifs.is_open() || !ProcessBegin())
  {
    return false;
  }

  m_Stats.m_Elements.Set(_("Files").ToStdString(), 1);
  
  std::string line;
  int line_no = 0;
  std::vector<std::string> v;

  while (std::getline(ifs, line))
  {
    if (!Process(line, line_no++)) return false;

    if (m_Write)
    {
      v.emplace_back(line);
    }
  }

  if (m_Write)
  {
    std::ofstream ofs(m_FileName.GetFullPath().c_str());
  
    for (const auto & it : v)
    {
      ofs << it << std::endl;
    }
  }
  
  ProcessEnd();

  return true;
}
