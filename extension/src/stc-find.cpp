////////////////////////////////////////////////////////////////////////////////
// Name:      stc-find.cpp
// Purpose:   Implementation of class wxExSTC Find methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/config.h>
#include <wx/log.h>
#include <wx/extension/stc.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/util.h>
#include <easylogging++.h>

bool wxExSTC::FindNext(bool find_next)
{
  return FindNext(
    GetFindString(),
    -1,
    find_next);
}

bool wxExSTC::FindNext(
  const std::string& text, 
  int find_flags,
  bool find_next)
{
  if (text.empty())
  {
    return false;
  }

  static bool recursive = false;
  static int start_pos, end_pos;
  const bool wrapscan(wxConfigBase::Get()->ReadLong(_("Wrap scan"), 1));

  if (find_next)
  {
    if (recursive) 
    {
      start_pos = 0;
      end_pos = GetCurrentPos();
    }
    else
    {
      start_pos = GetCurrentPos();
      end_pos = GetTextLength();
    }
  }
  else
  {
    if (recursive) 
    {
      start_pos = GetTextLength();
      end_pos = GetCurrentPos();
      if (GetSelectionStart() != -1)
        end_pos = GetSelectionStart();
    }
    else
    {
      start_pos = GetCurrentPos();
      if (GetSelectionStart() != -1)
        start_pos = GetSelectionStart();
      end_pos = 0;
    }
  }

  if (m_MarginTextClick >= 0)
  {
    bool found = false;
    static int line;
    std::match_results<std::string::const_iterator> m;

    if (find_next)
    {
      line = LineFromPosition(start_pos) + 1; 

      do
      {
        if (const std::string margin(MarginGetText(line));
           ((find_flags & wxSTC_FIND_REGEXP) && 
             std::regex_search(margin, m, std::regex(text))) ||
             margin.find(text) != std::string::npos) 
        {
          found = true;
        }
        else
        {
          line++;
        }
      } while (line <= LineFromPosition(end_pos) && !found);

      if (!found && !recursive && wrapscan)
      {
        recursive = true;
        found = FindNext(text, find_flags, find_next);
        recursive = false;
      }
    }
    else
    {
      line = LineFromPosition(start_pos) - 1; 

      do
      {
        if (const std::string margin(MarginGetText(line));
          ((find_flags & wxSTC_FIND_REGEXP) && 
            std::regex_search(margin, m, std::regex(text))) ||
            margin.find(text) != std::string::npos) 
        {
          found = true;
        }
        else
        {
          line--;
        }
      } while (line >= LineFromPosition(end_pos) && !found);

      if (!found && !recursive && wrapscan)
      {
        recursive = true;
        found = FindNext(text, find_flags, find_next);
        recursive = false;
      }
    }

    if (found)
    {
      wxExSTCData(wxExControlData().Line(line + 1), this).Inject();
      VLOG(9) << GetFileName().GetFullName() << 
        " found margin text: " << text << " on line: " << line + 1;
    }

    return found;
  }

  SetTargetStart(start_pos);
  SetTargetEnd(end_pos);
  SetSearchFlags(find_flags);

  if (SearchInTarget(text) == -1)
  {
    wxExFrame::StatusText(
      wxExGetFindResult(text, find_next, recursive), std::string());
    
    bool found = false;
    
    if (!recursive && wrapscan)
    {
      recursive = true;
      found = FindNext(text, find_flags, find_next);
      recursive = false;

      if (!found)
      {
        VLOG(9) << GetFileName().GetFullName() << " text: " << text << " not found";
      }
    }
    
    return found;
  }
  else
  {
    if (!recursive)
    {
      wxLogStatus(wxEmptyString);
    }
    
    recursive = false;

    if (m_vi.Mode().Normal() || m_vi.Mode().Insert())
    {
      SetSelection(GetTargetStart(), GetTargetEnd());
    }
    else if (m_vi.Mode().Visual())
    {
      if (find_next)
        m_vi.VisualExtend(GetSelectionStart(), GetTargetEnd());
      else
        m_vi.VisualExtend(GetTargetStart(), GetSelectionEnd());
    }
      
    EnsureVisible(LineFromPosition(GetTargetStart()));
    EnsureCaretVisible();

    VLOG(9) << GetFileName().GetFullName() << " found text: " << text;

    return true;
  }
}
