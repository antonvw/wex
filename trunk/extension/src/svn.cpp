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
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/log.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

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

void wxExSVN::Cleanup()
{
  delete m_STCEntryDialog;
}

bool wxExSVN::DirExists(const wxFileName& filename)
{
  wxFileName path(filename);
  path.AppendDir(".svn");
  return path.DirExists();
}

wxStandardID wxExSVN::Execute(wxWindow* parent)
{
  const wxString svn_flags_name = wxString::Format("svn/flags%d", m_Type);
  const wxString svn_flags_contents = wxConfigBase::Get()->Read(svn_flags_name);

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

    if (UseFlags())
    {
      wxConfigBase::Get()->Write(_("Flags"), svn_flags_contents);
      v.push_back(wxExConfigItem(_("Flags")));
    }

    if (UseSubcommand())
    {
      v.push_back(wxExConfigItem(_("Subcommand")));
    }

    m_ReturnCode = (wxStandardID)wxExConfigDialog(parent,
      v,
      m_Caption).ShowModal();

    if (m_ReturnCode == wxID_CANCEL)
    {
      return m_ReturnCode;
    }
  }

  const wxString cwd = wxGetCwd();

  wxString file;

  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(wxExConfigFirstOf(wxConfigBase::Get()->Read(_("Base folder"))));
  }
  else
  {
    file = " \"" + m_FullPath + "\"";
  }

  wxString comment;

  if (m_Type == SVN_COMMIT)
  {
    comment = 
      " -m \"" + wxExConfigFirstOf(wxConfigBase::Get()->Read(_("Revision comment"))) 
      + "\"";
  }

  wxString flags;
  wxString subcommand;
  
  if (UseSubcommand())
  {
    subcommand = wxConfigBase::Get()->Read(_("Subcommand"));

    if (!subcommand.empty())
    {
      subcommand = " " + subcommand;
    }
  }

  if (UseFlags())
  {
    flags = wxConfigBase::Get()->Read(_("Flags"));

    wxConfigBase::Get()->Write(svn_flags_name, flags);

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
    if (m_Output.empty())
    {
      m_Output = "Could not execute: " + command;
    }

    m_ReturnCode = wxID_ABORT;
    return m_ReturnCode;
  }

  wxExLog::Get()->Log(command);

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
  // We must have a parent.
  wxASSERT(parent != NULL);

  // If an error occurred, already shown by wxExecute itself.
  if (Execute(parent) == wxID_OK)
  {
    ShowOutput(parent);
  }

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
}

void wxExSVN::ShowOutput(wxWindow* parent) const
{
  switch (m_ReturnCode)
  {
    case wxID_CANCEL:
      break;

    case wxID_ABORT:
      wxMessageBox(m_Output);
      break;

    case wxID_OK:
    {
      const wxString caption = m_Caption +
        (!m_FullPath.empty() ? " " + 
            wxFileName(m_FullPath).GetFullName(): 
            wxString(wxEmptyString));

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
      // and there is a lexer.
      if (
        !m_FullPath.empty() &&
        (m_Type == SVN_CAT || m_Type == SVN_BLAME))
      {
        const wxExFileName fn(m_FullPath);
 
        if (!fn.GetLexer().GetScintillaLexer().empty())
        {
          m_STCEntryDialog->SetLexer(fn.GetLexer().GetScintillaLexer());
        }
      }

      m_STCEntryDialog->Show();
    }
    break;

    default:
      wxFAIL;
      break;
  }
}

bool wxExSVN::UseFlags() const
{
  return m_Type != SVN_UPDATE && m_Type != SVN_HELP;
}

bool wxExSVN::UseSubcommand() const
{
  return m_Type == SVN_HELP;
}

#endif
