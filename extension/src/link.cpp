////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/link.h>
#include <wx/extension/lexer.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

wxExLink::wxExLink(wxExSTC* stc)
  : m_STC(stc)
{
}

const wxString wxExLink::FindPath(const wxString& text) const
{
  if (
    text.empty() ||
    // wxPathList cannot handle links over several lines.
    // add trimmed argument, to skip eol
    wxExGetNumberOfLines(text, true) > 1)
  {
    return wxEmptyString;
  }

  // Path in .po files.
  if (
    m_STC != nullptr &&
    m_STC->GetLexer().GetScintillaLexer() == "po" && text.StartsWith("#: "))
  {
    return text.Mid(3);
  }

  // Better first try to find "...", then <...>, as in next example.
  // <A HREF="http://www.scintilla.org">scintilla</A> component.

  // So, first get text between " signs.
  size_t pos_char1 = text.find("\"");
  size_t pos_char2 = text.rfind("\"");

  // If that did not succeed, then get text between < and >.
  if (pos_char1 == wxString::npos || 
      pos_char2 == wxString::npos || 
      pos_char2 <= pos_char1)
  {
    pos_char1 = text.find("<");
    pos_char2 = text.rfind(">");
  }

  // If that did not succeed, then get text between ' and '.
  if (pos_char1 == wxString::npos ||
      pos_char2 == wxString::npos || 
      pos_char2 <= pos_char1)
  {
    pos_char1 = text.find("'");
    pos_char2 = text.rfind("'");
  }
  
  wxString out;

  // If we did not find anything.
  if (pos_char1 == wxString::npos || 
      pos_char2 == wxString::npos || 
      pos_char2 <= pos_char1)
  {
    out = text;
  }
  else
  {
    // Okay, get everything inbetween.
    out = text.substr(pos_char1 + 1, pos_char2 - pos_char1 - 1);
  }

  // And make sure we skip white space.
  out.Trim(true);
  out.Trim(false);
  
  return out;
}

const wxString wxExLink::GetPath(
  const wxString& text,
  int& line_no,
  int& column_no) const
{
  const wxString path(FindPath(text));
  wxString link(path);
  
  SetLink(link, line_no, column_no);
  
  if (
    link.empty() || 
    // Otherwise, if you happen to select text that 
    // ends with a separator, wx asserts.
    wxFileName::IsPathSeparator(link.Last()))
  {
    return wxEmptyString;
  }

  wxFileName file(link);
  wxString fullpath;

  if (file.FileExists())
  {
    file.MakeAbsolute();
    fullpath = file.GetFullPath();
  }
  else
  {
    if (
      file.IsRelative() && 
      m_STC != nullptr && 
      m_STC->GetFileName().FileExists())
    {
      if (file.MakeAbsolute(m_STC->GetFileName().GetPath()))
      {
        if (file.FileExists())
        {
          fullpath = file.GetFullPath();
        }
      }
    }

    if (fullpath.empty())
    {
      // Check whether last word is a file.
      wxString word = path.AfterLast(' ').Trim();
    
      if (
       !word.empty() && 
       !wxFileName::IsPathSeparator(link.Last()) &&
        wxFileExists(word))
      {
        wxFileName file(word);
        file.MakeAbsolute();
        fullpath = file.GetFullPath();
        // And reset line or column.
        line_no = 0;
        column_no = 0;
      }
    
      if (fullpath.empty() && !m_PathList.empty())
      {
        fullpath = m_PathList.FindAbsoluteValidPath(link);
      
        if (
          fullpath.empty() && 
         !word.empty() &&
          SetLink(word, line_no, column_no))
        {
          fullpath = m_PathList.FindAbsoluteValidPath(word);
        }
      }
      
      // Do nothing if fullpath.empty(),
      // as we return empty string if no path could be found.
    }
  }
  
  return fullpath;
}

bool wxExLink::SetLink(wxString& link, int& line_no, int& column_no) const
{
  if (link.size() < 2)
  {
    return false;
  }

  // Using backslash as separator does not yet work.
  link.Replace("\\\\", "/");
  link.Replace("\\", "/");

  // The harddrive letter is filtererd, it does not work
  // when adding it to wxExMatch.
  wxString prefix;

#ifdef __WXMSW__
  if (isalpha(link[0]) && link[1] == ':')
  {
    prefix = link.SubString(0,1);
    link = link.Mid(2);
  }
#endif

  // file[:line[:column]]
  std::vector <wxString> v;
  
  if (wxExMatch("([0-9A-Za-z _/.-]+):([0-9]*):?([0-9]*)", link.ToStdString(), v))
  {
    link = v[0];
    line_no = 0;
    column_no = 0;
      
    if (v.size() > 1)
    {
      line_no = atoi(v[1]);
        
      if (v.size() > 2)
      {
        column_no = atoi(v[2]);
      }
    }
      
    link = prefix + link;
    link.Trim();
    
    return true;
  }
  
  return false;
}
  
void wxExLink::SetFromConfig()
{
  wxStringTokenizer tkz(
    wxConfigBase::Get()->Read(_("Include directory")),
    "\r\n");
    
  m_PathList.Empty();
  
  while (tkz.HasMoreTokens())
  {
    m_PathList.Add(tkz.GetNextToken());
  }
}
