/******************************************************************************\
* File:          log.cpp
* Purpose:       Implementation of wxExLog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/textfile.h>
#include <wx/extension/log.h>

wxExLog::wxExLog(const wxString& filename)
  : m_FileName(filename)
{
}

bool wxExLog::Log(const wxString& text, bool add_timestamp ) const
{
  wxFile file(m_FileName, wxFile::write_append);

  return 
    file.IsOpened() &&
    file.Write((add_timestamp ? wxDateTime::Now().Format() + " ": "") 
      + text + wxTextFile::GetEOL());
}
