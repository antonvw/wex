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

  const wxString author = config->Get(_("Author"));
  const wxString address = config->Get(_("Address"));
  const wxString company = config->Get(_("Company"));
  const wxString country = config->Get(_("Country"));
  const wxString place = config->Get(_("Place"));
  const wxString zipcode = config->Get(_("Zipcode"));

  if (author.empty())
  {
    wxLogError("Author is required");
    return wxEmptyString;
  }

  const exLexer l = filename.GetLexer();

  header << l.MakeComment(wxEmptyString, true) << "\n";
  header << l.MakeComment("File:        " + filename.GetFullName(), true) << "\n";
  header << l.MakeCommentWithPrefix(description, 
                          "Purpose:     ") << "\n";
  header << l.MakeComment("Author:      " + author, true) << "\n";
  header << l.MakeComment("Created:     " + wxDateTime::Now().Format("%Y/%m/%d %H:%M:%S"), true) << "\n";

  if (config->GetBool("SVN"))
  // Prevent the Id to be expanded by SVN itself here.
  header << l.MakeComment("RCS-ID:      $" + wxString("Id$"), true) << "\n";

  header << l.MakeComment(wxEmptyString, true, true) << "\n";
  header << l.MakeComment("Copyright (c) " + wxDateTime::Now().Format("%Y") + " " + 
    (!company.empty() ? company: author), true) << "\n";

  if (!address.empty() && !country.empty() && !place.empty() && !zipcode.empty())
  {
    header << l.MakeComment(address + ", " + zipcode + " " + place + ", " + country, true) << "\n";
  }

  header << l.MakeComment("All rights reserved. Reproduction in whole or part is prohibited without", true) << "\n";
  header << l.MakeComment("the written consent of the copyright owner.", true) << "\n";
  header << l.MakeComment(wxEmptyString, true) << "\n";

  header << "\n";

  if (filename.GetExt() == "h" && 
      filename.GetStat().st_size == 0)
  {
    wxString argument = "__" + filename.GetName() + "_h";

    header << "\n";
    header << "#if !defined (" << argument << ")" << "\n";
    header << "#define " << argument << "\n";
    header << "#endif" << "\n";
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
