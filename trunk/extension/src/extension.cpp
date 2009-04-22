/******************************************************************************\
* File:          extension.cpp
* Purpose:       Implementation of wxWidgets extension methods
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/extension.h>

const wxString exHeader(
  const exFileName& filename,
  exConfig* config,
  const wxString& description)
{
  wxString header;

  const wxString author = config->Get("Header/Author");
  const wxString address = config->Get("Header/Address");
  const wxString company = config->Get("Header/Company");
  const wxString country = config->Get("Header/Country");
  const wxString place = config->Get("Header/Place");
  const wxString zipcode = config->Get("Header/Zipcode");

  const exLexer l = filename.GetLexer();

  header << l.MakeComment(wxEmptyString, true);
  header << l.MakeComment("File:        " + filename.GetFullName(), true);
// TODO: !!
//  WriteTextWithPrefix(description, "Purpose:     ");
  header << l.MakeComment("Author:      " + author, true);
  header << l.MakeComment("Created:     " + wxDateTime::Now().Format("%Y/%m/%d %H:%M:%S"), true);

  if (config->GetBool("SVN"))
  {
    // Prevent the Id to be expanded by SVN itself here.
    header << l.MakeComment("RCS-ID:      $" + wxString("Id$"), true);
  }

  header << l.MakeComment(wxEmptyString, true, true);
  header << l.MakeComment(
    "Copyright (c) " + wxDateTime::Now().Format("%Y") + 
    (!company.empty() ? " " + company: wxString(wxEmptyString))
      + ". All rights reserved.", true);

  if (!address.empty() && !country.empty() && !place.empty() && !zipcode.empty())
  {
    header << l.MakeComment(address + ", " + zipcode + " " + place + ", " + country, true);
  }

  header << l.MakeComment(wxEmptyString, true);

  header << "\n";

  if (filename.GetExt() == "h" && 
      filename.GetStat().st_size == 0)
  {
    wxString argument = "__" + filename.GetName() + "_h";

    header << "\n";
    header << "#if !defined (" << argument << ")";
    header << "#define " << argument;
    header << "#endif";
  }

  return header;
}

void exOpenFile(const wxFileName& filename, long open_flags)
{
  if (wxTheApp == NULL)
  {
    return;
  }

  wxWindow* window = wxTheApp->GetTopWindow();
  exFrame* frame = wxDynamicCast(window, exFrame);

  if (frame != NULL)
  {
    frame->OpenFile(exFileName(filename.GetFullPath()), -1, wxEmptyString, open_flags);
  }
}
