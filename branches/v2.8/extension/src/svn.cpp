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

#include <wx/config.h>
#include <wx/extension/svn.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>

wxExSVN* wxExSVN::m_Self = NULL;
#if wxUSE_GUI
wxExSTCEntryDialog* wxExSVN::m_STCEntryDialog = NULL;
#endif
wxString wxExSVN::m_UsageKey;

wxExSVN::wxExSVN()
  : m_Type(SVN_NONE)
  , m_FullPath(wxEmptyString)
{
  Initialize();
}

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

#if wxUSE_GUI
int wxExSVN::ConfigDialog(
  wxWindow* parent,
  const wxString& title) const
{
  std::vector<wxExConfigItem> v;
  v.push_back(wxExConfigItem()); // a spacer
  v.push_back(wxExConfigItem(m_UsageKey, CONFIG_CHECKBOX));
  v.push_back(wxExConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL));

  return wxExConfigDialog(parent, v, title).ShowModal();
}
#endif

bool wxExSVN::DirExists(const wxFileName& filename) const
{
  wxFileName path(filename);
  path.AppendDir(".svn");
  return path.DirExists();
}

long wxExSVN::Execute()
{
  wxASSERT(m_Type != SVN_NONE);

  const wxString cwd = wxGetCwd();

  wxString file;

  if (m_FullPath.empty())
  {
    if (!wxSetWorkingDirectory(wxExConfigFirstOf(_("Base folder"))))
    {
      m_Output = _("Cannot set working directory");
      return -1;
    }

    if (m_Type == SVN_ADD)
    {
      file = " " + wxExConfigFirstOf(_("Path"));
    }
  }
  else
  {
    file = " \"" + m_FullPath + "\"";
  }

  wxString comment;

  if (m_Type == SVN_COMMIT)
  {
    comment = 
      " -m \"" + wxExConfigFirstOf(_("Revision comment")) + "\"";
  }

  wxString subcommand;
  
  if (UseSubcommand())
  {
    subcommand = wxConfigBase::Get()->Read(_("Subcommand"));

    if (!subcommand.empty())
    {
      subcommand = " " + subcommand;
    }
  }

  wxString flags;

  if (UseFlags())
  {
    flags = wxConfigBase::Get()->Read(_("Flags"));

    if (!flags.empty())
    {
      flags = " " + flags;
    }
  }

  m_CommandWithFlags = m_Command + flags;

  const wxString commandline = 
    "svn " + m_Command + subcommand + flags + comment + file;

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(commandline);
#endif

  wxArrayString output;
  wxArrayString errors;
  long retValue;

  if ((retValue = wxExecute(
    commandline,
    output,
    errors)) == -1)
  {
    // See also process, same log is shown.
    wxLogError(_("Cannot execute") + ": " + commandline);
  }
  else
  {
    wxExLog::Get()->Log(commandline);
  }

  if (m_FullPath.empty())
  {
    wxSetWorkingDirectory(cwd);
  }

  m_Output.clear();

  // First output the errors.
  for (
    size_t i = 0;
    i < errors.GetCount();
    i++)
  {
    m_Output += errors[i] + "\n";
  }

  // Then the normal output, will be empty if there are errors.
  for (
    size_t j = 0;
    j < output.GetCount();
    j++)
  {
    m_Output += output[j] + "\n";
  }

  return retValue;
}

#if wxUSE_GUI
int wxExSVN::Execute(wxWindow* parent)
{
  wxASSERT(parent != NULL);

  // Key SVN is already used, so use other name.
  const wxString svn_flags_name = wxString::Format("svnflags/name%d", m_Type);

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

    if (m_Type == SVN_ADD)
    {
      v.push_back(wxExConfigItem(
        _("Path"), 
        CONFIG_COMBOBOX,
        wxEmptyString, 
        true)); // required
    }
  }

  if (UseFlags())
  {
    wxConfigBase::Get()->Write(
      _("Flags"), 
      wxConfigBase::Get()->Read(svn_flags_name));

    v.push_back(wxExConfigItem(_("Flags")));
  }

  if (UseSubcommand())
  {
    v.push_back(wxExConfigItem(_("Subcommand")));
  }

  if (wxExConfigDialog(parent,
    v,
    m_Caption).ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  if (UseFlags())
  {
    wxConfigBase::Get()->Write(svn_flags_name, 
      wxConfigBase::Get()->Read(_("Flags")));
  }

  Execute();
  
  return wxID_OK;
}
#endif

#if wxUSE_GUI
int wxExSVN::ExecuteAndShowOutput(wxWindow* parent)
{
  int retValue;

  if ((retValue = Execute(parent)) == wxID_OK)
  {
    if (!m_Output.empty())
    {
      ShowOutput(parent);
    }
  }

  return retValue;
}
#endif

wxExSVN* wxExSVN::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExSVN;

    if (!wxConfigBase::Get()->Exists(m_UsageKey))
    {
      wxConfigBase::Get()->Write(m_UsageKey, true);
    }
  }

  return m_Self;
}

wxExSVNType wxExSVN::GetType(int command_id) const
{
  switch (command_id)
  {
    case ID_EDIT_SVN_ADD: return SVN_ADD; break;
    case ID_EDIT_SVN_BLAME: return SVN_BLAME; break;
    case ID_EDIT_SVN_CAT: return SVN_CAT; break;
    case ID_EDIT_SVN_COMMIT: return SVN_COMMIT; break;
    case ID_EDIT_SVN_DIFF: return SVN_DIFF; break;
    case ID_EDIT_SVN_HELP: return SVN_HELP; break;
    case ID_EDIT_SVN_INFO: return SVN_INFO; break;
    case ID_EDIT_SVN_LOG: return SVN_LOG; break;
    case ID_EDIT_SVN_PROPLIST: return SVN_PROPLIST; break;
    case ID_EDIT_SVN_PROPSET: return SVN_PROPSET; break;
    case ID_EDIT_SVN_REVERT: return SVN_REVERT; break;
    case ID_EDIT_SVN_STAT: return SVN_STAT; break;
    case ID_EDIT_SVN_UPDATE: return SVN_UPDATE; break;
    default:
      wxFAIL;
      return SVN_NONE;
      break;
  }
}

void wxExSVN::Initialize()
{
  switch (m_Type)
  {
    case SVN_NONE:     m_Caption = ""; break;
    case SVN_ADD:      m_Caption = "SVN Add"; break;
    case SVN_BLAME:    m_Caption = "SVN Blame"; break;
    case SVN_CAT:      m_Caption = "SVN Cat"; break;
    case SVN_COMMIT:   m_Caption = "SVN Commit"; break;
    case SVN_DIFF:     m_Caption = "SVN Diff"; break;
    case SVN_HELP:     m_Caption = "SVN Help"; break;
    case SVN_INFO:     m_Caption = "SVN Info"; break;
    case SVN_LOG:      m_Caption = "SVN Log"; break;
    case SVN_LS:       m_Caption = "SVN Ls"; break;
    case SVN_PROPLIST: m_Caption = "SVN Proplist"; break;
    case SVN_PROPSET:  m_Caption = "SVN Propset"; break;
    case SVN_REVERT:   m_Caption = "SVN Revert"; break;
    case SVN_STAT:     m_Caption = "SVN Stat"; break;
    case SVN_UPDATE:   m_Caption = "SVN Update"; break;
    default:
      wxFAIL;
      break;
  }

  m_Command = m_Caption.AfterFirst(' ').Lower();

  // Currently no flags, as no command was executed.
  m_CommandWithFlags = m_Command;

  m_Output.clear();
  m_UsageKey = _("Use SVN");
}

wxExSVN* wxExSVN::Set(wxExSVN* svn)
{
  wxExSVN* old = m_Self;
  m_Self = svn;
  return old;
}

#if wxUSE_GUI
void wxExSVN::ShowOutput(wxWindow* parent) const
{
  wxString caption = m_Caption;
      
  if (m_Type != SVN_HELP)
  {
    caption += " " + (!m_FullPath.empty() ?  
      wxFileName(m_FullPath).GetFullName(): 
      wxExConfigFirstOf(_("Base folder")));
  }

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
#endif

bool wxExSVN::Use() const
{
  return wxConfigBase::Get()->Read(m_UsageKey, true);
}

bool wxExSVN::UseFlags() const
{
  return m_Type != SVN_UPDATE && m_Type != SVN_HELP;
}

bool wxExSVN::UseSubcommand() const
{
  return m_Type == SVN_HELP;
}
