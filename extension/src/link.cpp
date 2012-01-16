////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
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

bool wxExLink::AddBasePath()
{
  // Find the base path, if this is not yet on the list, add it.
  const wxString basepath_text = "Basepath: ";
  const int find = m_STC->FindText(
    0,
    1000, // the max pos to look for, this seems enough
    basepath_text);

  if (find == -1)
  {
    return false;
  }
  
  const int line = m_STC->LineFromPosition(find);

  m_PathList.Add(m_STC->GetTextRange(
    find + basepath_text.length(),
    m_STC->GetLineEndPosition(line) - 3));
    
  return true;
}

const wxString wxExLink::FindPath(const wxString& text) const
{
  if (
    text.empty() ||
    // wxPathList cannot handle links over several lines.
    wxExGetNumberOfLines(text) > 1)
  {
    return wxEmptyString;
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

  // If that did not succeed, then get text between : and : (in .po files).
  if (m_STC->GetLexer().GetScintillaLexer() == "po" && 
      (pos_char1 == wxString::npos || 
       pos_char2 == wxString::npos || 
       pos_char2 <= pos_char1))
  {
    pos_char1 = text.find(": ");
    pos_char2 = text.rfind(":");
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
    // Filter out a possible line number, or ending with : and whitespace.
    try {
      std::match_results<std::string::const_iterator> res;
      std::regex rx("(.+):{[0-9]+}| *");
      std::string str(text.c_str());
      std::regex_search(str, res, rx);
    
      if (res.size() > 0)
      {
        out = wxString(res[1]);
      }
      else
      {
        out = text;
      }
      }
      catch(std::exception& e) {
        wxLogMessage(e.what());
      }
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

int wxExLink::GetLineNo(const wxString& text) const
{
  return wxExGetLineNumber(text);
}

const wxString wxExLink::GetPath(const wxString& text) const
{
  const wxString link(FindPath(text));

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
    if (file.IsRelative())
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
      fullpath = m_PathList.FindAbsoluteValidPath(link);
    }
  }
  
  return fullpath;
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
