/******************************************************************************\
* File:          svn.cpp
* Purpose:       Implementation of wxExSVN class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/svn.h>
#include <wx/extension/app.h> // for wxExApp
#include <wx/extension/configdialog.h>
#include <wx/extension/defs.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

wxExSTCEntryDialog* wxExSVN::m_STCEntryDialog = NULL;

wxExSVN::wxExSVN(int command_id, const wxString& fullpath)
  : m_Type(GetType(command_id))
  , m_FullPath(fullpath)
{
  Initialize();
}

wxExSVN::wxExSVN(wxExSVNType type, const wxString& fullpath)
  : m_Type(type)
  , m_FullPath(fullpath)
{
  Initialize();
}

wxStandardID wxExSVN::Execute(wxWindow* parent)
{
  const wxString svn_flags_name = wxString::Format("svn/flags%d", m_Type);
  const wxString svn_flags_contents = wxExApp::GetConfig(svn_flags_name);

  if (parent != NULL)
  {
    std::vector<wxExConfigItem> v;

    if (m_Type == SVN_COMMIT)
    {
      v.push_back(wxExConfigItem(
        _("Revision comment"), 
        CONFIG_COMBOBOX,
        wxEmptyString,
        true)); // required
    }

    if (m_FullPath.empty() && m_Type != SVN_HELP)
    {
      v.push_back(wxExConfigItem(
        _("Base folder"), 
        CONFIG_COMBOBOXDIR, 
        wxEmptyString, 
        true)); // required
    }

    // SVN_UPDATE and SVN_HELP have no flags to ask for.
    if (m_Type != SVN_UPDATE && m_Type != SVN_HELP)
    {
      wxExApp::SetConfig(_("Flags"), svn_flags_contents);
      v.push_back(wxExConfigItem(_("Flags")));
    }

    // Instead, SVN_HELP has an extra subcommand.
    if (m_Type == SVN_HELP)
    {
      v.push_back(wxExConfigItem(_("Subcommand")));
    }

    if (wxExConfigDialog(parent,
      wxExApp::GetConfig(),
      v,
      m_Caption).ShowModal() == wxID_CANCEL)
    {
      m_ReturnCode = wxID_CANCEL;
      return m_ReturnCode;
    }
  }

  const wxString cwd = wxGetCwd();

  wxString file;

  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(wxExApp::GetConfig(_("Base folder")));
  }
  else
  {
    file = " \"" + m_FullPath + "\"";
  }

  wxString comment;

  if (m_Type == SVN_COMMIT)
  {
    comment = " -m \"" + wxExApp::GetConfig(_("Revision comment")) + "\"";
  }

  wxString flags;
  wxString subcommand;
  
  if (m_Type == SVN_HELP)
  {
    subcommand = wxExApp::GetConfig(_("Subcommand"));

    if (!subcommand.empty())
    {
      subcommand = " " + subcommand;
    }
  }
  else
  {
    flags = wxExApp::GetConfig(_("Flags"));

    wxExApp::SetConfig(svn_flags_name, flags);

    m_CommandWithFlags = m_Command + " " + flags;

    if (!flags.empty())
    {
      flags += " ";
    }
  }

  const wxString command = 
    "svn " + flags + m_Command + subcommand + comment + file;

  wxArrayString output;
  wxArrayString errors;
  m_Output.clear();

  if (wxExecute(
    command,
    output,
    errors) == -1)
  {
    m_ReturnCode = wxID_ABORT;
    return m_ReturnCode;
  }

  wxExApp::Log(command);

  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(cwd);
  }

  // First output the errors.
  for (size_t i = 0; i < errors.GetCount(); i++)
  {
    m_Output += errors[i] + "\n";
  }

  // Then the normal output, will be empty if there are errors.
  for (size_t j = 0; j < output.GetCount(); j++)
  {
    m_Output += output[j] + "\n";
  }

  return wxID_OK;
}

wxStandardID wxExSVN::ExecuteAndShowOutput(wxWindow* parent)
{
  Execute(parent);
  ShowOutput(parent);
  return m_ReturnCode;
}

wxExSVNType wxExSVN::GetType(int command_id) const
{
  switch (command_id)
  {
    case ID_EDIT_SVN_BLAME: return SVN_BLAME; break;
    case ID_EDIT_SVN_CAT: return SVN_CAT; break;
    case ID_EDIT_SVN_COMMIT: return SVN_COMMIT; break;
    case ID_EDIT_SVN_DIFF: return SVN_DIFF; break;
    case ID_EDIT_SVN_HELP: return SVN_HELP; break;
    case ID_EDIT_SVN_INFO: return SVN_INFO; break;
    case ID_EDIT_SVN_LOG: return SVN_LOG; break;
    case ID_EDIT_SVN_REVERT: return SVN_REVERT; break;
    case ID_EDIT_SVN_STAT: return SVN_STAT; break;
    case ID_EDIT_SVN_UPDATE: return SVN_UPDATE; break;
    default:
      wxFAIL;
      return SVN_STAT;
      break;
  }
}

void wxExSVN::Initialize()
{
  switch (m_Type)
  {
    case SVN_BLAME:  m_Caption = "SVN Blame"; break;
    case SVN_CAT:    m_Caption = "SVN Cat"; break;
    case SVN_COMMIT: m_Caption = "SVN Commit"; break;
    case SVN_DIFF:   m_Caption = "SVN Diff"; break;
    case SVN_HELP:   m_Caption = "SVN Help"; break;
    case SVN_INFO:   m_Caption = "SVN Info"; break;
    case SVN_LOG:    m_Caption = "SVN Log"; break;
    case SVN_REVERT: m_Caption = "SVN Revert"; break;
    case SVN_STAT:   m_Caption = "SVN Stat"; break;
    case SVN_UPDATE: m_Caption = "SVN Update"; break;
    default:
      wxFAIL;
      break;
  }

  m_Command = m_Caption.AfterFirst(' ').Lower();

  // Currently no flags, as no command was executed.
  m_CommandWithFlags = m_Command;

  m_Output.clear();
  m_ReturnCode = wxID_NONE;

  wxASSERT(wxExApp::GetConfig() != NULL);
}

void wxExSVN::ShowOutput(wxWindow* parent) const
{
  if (m_ReturnCode != wxID_OK)
  {
    return;
  }

  const wxString caption = m_Caption +
    (!m_FullPath.empty() ? " " + wxFileName(m_FullPath).GetFullName(): wxString(wxEmptyString));

  // Create a dialog for contents.
  if (m_STCEntryDialog == NULL)
  {
    m_STCEntryDialog = new wxExSTCEntryDialog(
      parent,
      caption,
      m_Output,
      wxEmptyString,
      wxOK,
      wxID_ANY,
      wxDefaultPosition, wxSize(575, 250));
  }
  else
  {
    m_STCEntryDialog->SetText(m_Output);
    m_STCEntryDialog->SetTitle(caption);

    // Reset a previous lexer.
    if (!m_STCEntryDialog->GetLexer().empty())
    {
      m_STCEntryDialog->SetLexer(wxEmptyString);
    }
  }

  // Add a lexer if we specified a path, asked for cat or blame 
  // and there were no errors, and there is a lexer.
  if (
    !m_FullPath.empty() &&
    (m_Type == SVN_CAT || m_Type == SVN_BLAME) &&
     m_ReturnCode == 0)
  {
    const wxExFileName fn(m_FullPath);

    if (!fn.GetLexer().GetScintillaLexer().empty())
    {
      m_STCEntryDialog->SetLexer(fn.GetLexer().GetScintillaLexer());
    }
  }

  m_STCEntryDialog->Show();
}

#endif
