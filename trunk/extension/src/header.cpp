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

#include <wx/config.h>
#include <wx/extension/header.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/vcs.h>
#include <wx/extension/util.h>

wxExHeader::wxExHeader()
  : m_TextAuthor(_("Author"))
  , m_TextEmail(_("Email"))
  , m_TextLicense(_("License"))
  , m_TextPurpose(_("Purpose"))
{
}

const wxString wxExHeader::Get(const wxExFileName* filename) const
{
  const wxString h_name      = "Name:      "; 
  const wxString h_purpose   = "Purpose:   "; 
  const wxString h_author    = "Author:    "; 
  const wxString h_created   = "Created:   "; 
  const wxString h_rcs       = "RCS-ID:    $"; 
  const wxString h_copyright = "Copyright: "; 
  const wxString h_license   = "License:   "; 

  const wxString author = wxConfigBase::Get()->Read(m_TextAuthor);
  const wxString license = wxConfigBase::Get()->Read(m_TextLicense);
  const wxString email = wxConfigBase::Get()->Read(m_TextEmail);
  const wxString purpose = wxConfigBase::Get()->Read(m_TextPurpose);

  const wxString email_field = (!email.empty() ? " < " + email + ">": email);

  wxString header;
  const wxExLexer l = filename->GetLexer();

  if (!l.GetCommentEnd().empty())
  {
    header << l.GetCommentBegin() << "\n";
    header << h_name << filename->GetFullName() << "\n";
    header << wxExAlignText(purpose, h_purpose) << "\n";
    header << h_author << author << "\n";
    header << h_created << wxDateTime::Now().FormatISODate() << "\n";
    if (wxExVCS::Get()->GetVCS() == wxExVCS::VCS_SVN)
    // Prevent the Id to be expanded by VCS itself here.
    header << h_rcs << wxString("Id$") << "\n";
    header << h_copyright << "(c) " << wxDateTime::Now().Format("%Y") << " " <<
      author << email_field << "\n";
    if (!license.empty())
    header << h_license << license << "\n";
    header << l.GetCommentEnd() << "\n";
  }
  else
  {
    header << l.MakeComment(wxEmptyString, false) << "\n";
    header << l.MakeComment(h_name, filename->GetFullName()) << "\n";
    header << l.MakeComment(h_purpose, purpose) << "\n";
    header << l.MakeComment(h_author, author) << "\n";
    header << l.MakeComment(h_created, wxDateTime::Now().FormatISODate()) << "\n";
    if (wxExVCS::Get()->GetVCS() == wxExVCS::VCS_SVN)
    // Prevent the Id to be expanded by VCS itself here.
    header << l.MakeComment(h_rcs, wxString("Id$")) << "\n";
    header << l.MakeComment(h_copyright, "(c) " + wxDateTime::Now().Format("%Y") + " " +
      author + email_field) << "\n";
    if (!license.empty())
    header << l.MakeComment(h_license, license) << "\n";
    header << l.MakeComment(wxEmptyString, false) << "\n";
  }

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
  v.push_back(wxExConfigItem(m_TextPurpose, wxEmptyString, wxTE_MULTILINE, true));

  // Author is required, but only presented if empty.
  // Email and License also are only presented if Author empty.
  if (wxConfigBase::Get()->Read(m_TextAuthor).empty())
  {
    v.push_back(wxExConfigItem(m_TextAuthor, wxEmptyString, 0, true));

    if (wxConfigBase::Get()->Read(m_TextEmail).empty())
    {
      v.push_back(wxExConfigItem(m_TextEmail));
    }

    if (wxConfigBase::Get()->Read(m_TextLicense).empty())
    {
      v.push_back(wxExConfigItem(m_TextLicense));
    }
  }

  return wxExConfigDialog(
    parent, 
    v, title).ShowModal();
}
