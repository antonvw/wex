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
#include <wx/extension/configdialog.h>

const wxString wxExHeader(
  const wxExFileName& filename,
  wxExConfig* config)
{
  wxString header;

  const wxString author = config->Get(_("Author"));
  const wxString address = config->Get(_("Address"));
  const wxString company = config->Get(_("Company"));
  const wxString country = config->Get(_("Country"));
  const wxString license = config->Get(_("License"));
  const wxString email = config->Get(_("Email"));
  const wxString place = config->Get(_("Place"));
  const wxString purpose = config->Get(_("Purpose"));
  const wxString zipcode = config->Get(_("Zipcode"));

  const wxString email_field = (!email.empty() ? " < " + email + ">": email);

  if (author.empty())
  {
    wxLogError("Author is required");
    return wxEmptyString;
  }

  const wxExLexer l = filename.GetLexer();

  if (l.GetScintillaLexer().empty())
  {
    wxLogError("Lexer is empty");
    return wxEmptyString;
  }

  header << l.MakeComment(wxEmptyString, false) << "\n";
  header << l.MakeComment("File:       ", filename.GetFullName()) << "\n";
  header << l.MakeComment("Purpose:    ", purpose) << "\n";
  header << l.MakeComment("Author:     ", author) << "\n";
  header << l.MakeComment("Created:    ", wxDateTime::Now().Format("%Y/%m/%d %H:%M:%S")) << "\n";

  if (config->GetBool("SVN"))
  // Prevent the Id to be expanded by SVN itself here.
  header << l.MakeComment("RCS-ID:     $", wxString("Id$")) << "\n";
  if (!license.empty())
  header << l.MakeComment("License:    ", license) << "\n";

  header << l.MakeComment(wxEmptyString) << "\n";
  header << l.MakeComment("Copyright (c) " + wxDateTime::Now().Format("%Y") + " " +
    (!company.empty() ? company: author) + email_field) << "\n";

  if (!address.empty() && !country.empty() && !place.empty() && !zipcode.empty())
  {
    header << l.MakeComment(address + ", " + zipcode + " " + place + ", " + country) << "\n";
  }

  header << l.MakeComment("\
    All rights reserved. Reproduction in whole or part is prohibited without the\
     written consent of the copyright owner.") << "\n";

  header << l.MakeComment(wxEmptyString, false) << "\n";

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

int wxExHeaderDialog(wxWindow* parent, wxExConfig* config)
{
  std::vector<wxExConfigItem> v;

  // Purpose is required.
  v.push_back(wxExConfigItem(_("Purpose"), wxEmptyString, wxTE_MULTILINE, true));

  // Author is required, but only presented if empty.
  // Email and License also are only presented if Author empty.
  if (config->Get(_("Author")).empty())
  {
    v.push_back(wxExConfigItem(_("Author"), wxEmptyString, 0, true));

    if (config->Get(_("Email")).empty())
    {
      v.push_back(wxExConfigItem(_("Email")));
    }

    if (config->Get(_("License")).empty())
    {
      v.push_back(wxExConfigItem(_("License")));
    }
  }

  return wxExConfigDialog(
    parent, 
    config, v, _("File Purpose")).ShowModal();
}

void wxExOpenFile(const wxFileName& filename, long open_flags)
{
  wxASSERT(wxTheApp != NULL);

  wxWindow* window = wxTheApp->GetTopWindow();
  wxExFrame* frame = wxDynamicCast(window, wxExFrame);

  if (frame != NULL)
  {
    frame->OpenFile(wxExFileName(filename.GetFullPath()), -1, wxEmptyString, open_flags);
  }
}
