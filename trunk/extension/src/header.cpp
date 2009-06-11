/******************************************************************************\
* File:          header.cpp
* Purpose:       Implementation of wxExHeader class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/header.h>
#include <wx/extension/configdialog.h>
#include <wx/extension/file.h>

wxExHeader::wxExHeader(wxExConfig* config)
  : m_Config(config)
{
}

const wxString wxExHeader::Get(const wxExFileName* filename) const
{
  const wxExLexer l = filename->GetLexer();

  if (l.GetScintillaLexer().empty())
  {
    wxLogError("Lexer is empty");
    return wxEmptyString;
  }

  const wxString author = m_Config->Get(_("Author"));
  const wxString company = m_Config->Get(_("Company"));
  const wxString license = m_Config->Get(_("License"));
  const wxString email = m_Config->Get(_("Email"));
  const wxString purpose = m_Config->Get(_("Purpose"));
  const wxString email_field = (!email.empty() ? " < " + email + ">": email);

  wxString header;
  header << l.MakeComment(wxEmptyString, false) << "\n";
  header << l.MakeComment("Name:       ", filename->GetFullName()) << "\n";
  header << l.MakeComment("Purpose:    ", purpose) << "\n";
  header << l.MakeComment("Author:     ", author) << "\n";
  header << l.MakeComment("Created:    ", wxDateTime::Now().FormatISODate()) << "\n";
  if (m_Config->GetBool("SVN"))
  // Prevent the Id to be expanded by SVN itself here.
  header << l.MakeComment("RCS-ID:     $", wxString("Id$")) << "\n";
  header << l.MakeComment("Copyright:   (c) " + wxDateTime::Now().Format("%Y") + " " +
    (!company.empty() ? company: author) + email_field) << "\n";
  if (!license.empty())
  header << l.MakeComment("License:    ", license) << "\n";
  header << l.MakeComment(wxEmptyString, false) << "\n";

  header << "\n";

  if (filename->GetExt() == "h" &&
      filename->GetStat().st_size == 0)
  {
    wxString argument = "__" + filename->GetName() + "_h";

    header << "\n";
    header << "#if !defined (" << argument << ")" << "\n";
    header << "#define " << argument << "\n";
    header << "#endif" << "\n";
  }

  return header;
}

int wxExHeader::ShowDialog(wxWindow* parent, const wxString& title) const
{
  std::vector<wxExConfigItem> v;

  // Purpose is required.
  v.push_back(wxExConfigItem(_("Purpose"), wxEmptyString, wxTE_MULTILINE, true));

  // Author is required, but only presented if empty.
  // Email and License also are only presented if Author empty.
  if (m_Config->Get(_("Author")).empty())
  {
    v.push_back(wxExConfigItem(_("Author"), wxEmptyString, 0, true));

    if (m_Config->Get(_("Email")).empty())
    {
      v.push_back(wxExConfigItem(_("Email")));
    }

    if (m_Config->Get(_("License")).empty())
    {
      v.push_back(wxExConfigItem(_("License")));
    }
  }

  return wxExConfigDialog(
    parent, 
    m_Config, v, title).ShowModal();
}
