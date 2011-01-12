////////////////////////////////////////////////////////////////////////////////
// Name:      filename.cpp
// Purpose:   Implementation of class 'wxExFileName'
// Author:    Anton van Wezenbeek
// Created:   2010-03-18
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/filename.h>
#include <wx/extension/lexers.h>

wxExFileName::wxExFileName(const wxString& fullpath, wxPathFormat format)
  : wxFileName(fullpath, format)
  , m_Stat(fullpath) 
{
  // This construct prevents an assert in the test base.
  wxExLexers* lexers = wxExLexers::Get(false);

  if (lexers != NULL)
  {
    m_Lexer = lexers->FindByFileName(*this);
  }
}

wxExFileName::wxExFileName(const wxFileName& filename)
  : wxFileName(filename)
  , m_Stat(filename.GetFullPath()) 
{
  wxExLexers* lexers = wxExLexers::Get(false);

  if (lexers != NULL)
  {
    m_Lexer = lexers->FindByFileName(*this);
  }
}

void wxExFileName::StatusText(long flags) const
{
  // Clear status bar for empty or not existing or not initialized file names.
  wxString text; 

  if (IsOk())
  {
    const wxString path = (flags & STAT_FULLPATH
      ? GetFullPath(): GetFullName());

    text += path;

    if (GetStat().IsOk())
    {
      const wxString what = (flags & STAT_SYNC
        ? _("Synchronized"): _("Modified"));
      const wxString time = (flags & STAT_SYNC
        ? wxDateTime::Now().Format(): GetStat().GetModificationTime());
      text += " " + what + " " + time;
    }
  }

  wxLogStatus(text);
}
