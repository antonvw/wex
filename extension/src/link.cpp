////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/link.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

wxExLink::wxExLink(wxExSTC* stc)
  : m_STC(stc)
{
}

void wxExLink::AddBasePath()
{
  // First find the base path, if this is not yet on the list, add it.
  const wxString basepath_text = "Basepath:";

  const int find = m_STC->FindText(
    0,
    1000, // the max pos to look for, this seems enough
    basepath_text,
    wxSTC_FIND_WHOLEWORD);

  if (find == -1)
  {
    return;
  }

  const int  line = m_STC->LineFromPosition(find);
  const wxString basepath = m_STC->GetTextRange(
    find + basepath_text.length() + 1,
    m_STC->GetLineEndPosition(line) - 3);

  m_PathList.Add(basepath);
}

const wxString wxExLink::GetPath(const wxString& line) const
{
  // Any line info is already in line_number, so skip here.
  const wxString no = line.AfterFirst(':');
  const wxString link = line.BeforeFirst(':');

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
  
const wxString wxExLink::GetTextAtCurrentPos() const
{
  const wxString sel = m_STC->GetSelectedText();

  if (!sel.empty())
  {
    if (wxExGetNumberOfLines(sel) > 1)
    {
      // wxPathList cannot handle links over several lines.
      return wxEmptyString;
    }

    return sel;
  }
  else
  {
    const int pos = m_STC->GetCurrentPos();
    const int line_no = m_STC->LineFromPosition(pos);
    const wxString text = m_STC->GetLine(line_no);

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
    if (pos_char1 == wxString::npos || 
        pos_char2 == wxString::npos || 
        pos_char2 <= pos_char1)
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
