////////////////////////////////////////////////////////////////////////////////
// Name:      textfile.cpp
// Purpose:   Implementation of wxExTextFile class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <wx/extension/lexers.h>
#include <wx/extension/textfile.h>
#include <wx/extension/frd.h>

wxExTextFile::wxExTextFile(
  const wxExFileName& filename,
  const wxExTool& tool)
  : m_FileName(filename)
  , m_Tool(tool)
  , m_Modified(false)
{
}

bool wxExTextFile::MatchLine(wxString& line)
{
  bool match = false;
  int count = 1;

  wxExFindReplaceData* frd = wxExFindReplaceData::Get();

  if (!frd->UseRegEx())
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_FIND)
    {
      std::string search_line(line);

      if (!frd->MatchCase())
      {
        std::transform(
          search_line.begin(), 
          search_line.end(), 
          search_line.begin(), 
          toupper);
      }

      const size_t start = search_line.find(m_FindString);

      if (start != wxString::npos)
      {
        if (frd->MatchWord())
        {
          if (( start == 0 ||
               (start > 0 && !IsWordCharacter(search_line[start - 1]))) &&
              !IsWordCharacter(search_line[start + m_FindString.length()]))
          {
            match = true;
          }
        }
        else
        {
          match = true;
        }
      }
    }
    else
    {
      count = line.Replace(
        frd->GetFindString(), 
        frd->GetReplaceString());

      if (count > 0)
      {
        m_Modified = true;
        match = true;
      }
    }
  }
  else
  {
    match = frd->RegExMatches(line.ToStdString());

    if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
    {
      std::string s(line);
      count = frd->RegExReplaceAll(s);
      line = s;
      m_Modified = true;
    }
  }

  if (match)
  {
    IncActionsCompleted(count);
  }

  return match;
}

bool wxExTextFile::Parse()
{
  if (
    !m_Tool.IsFindType() || 
    (m_Tool.GetId() == ID_TOOL_REPORT_REPLACE && m_FileName.GetStat().IsReadOnly()))
  {
    return false;
  }

  if (wxExFindReplaceData::Get()->GetFindString().empty())
  {
    return false;
  }

  m_FindString = wxExFindReplaceData::Get()->GetFindString();

  if (!wxExFindReplaceData::Get()->MatchCase())
  {
    std::transform(
      m_FindString.begin(), 
      m_FindString.end(), 
      m_FindString.begin(), 
      toupper);
  }
  
  const int prev = m_Stats.Get(_("Actions Completed"));

  for (size_t i = 0; i < GetLineCount(); i++)
  {
    wxString& line = GetLine(i);

    if (MatchLine(line))
    {
      Report(i);
      
      if (m_Stats.Get(_("Actions Completed")) - prev > 250)
      {
        wxLogMessage("too many matches, reconsider your search");
        return false;
      }
    }
  }

  return true;
}

bool wxExTextFile::RunTool()
{
  if (!wxTextFile::Open(m_FileName.GetFullPath()))
  {
    return false;
  }

  m_Stats.m_Elements.Set(_("Files"), 1);

  if (GetLineCount() > 0)
  {
    if (!m_FileName.GetLexer().IsOk())
    {
      m_FileName.SetLexer(wxExLexers::Get()->FindByText(GetLine(0).ToStdString()));
    }

    if (!Parse())
    {
      Close();
      return false;
    }
  }

  if (m_Modified && !m_FileName.GetStat().IsReadOnly())
  {
    if (!Write())
    {
      Close();
      return false;
    }
  }

  Close();

  return true;
}
