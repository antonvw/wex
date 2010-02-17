/******************************************************************************\
* File:          vcs.cpp
* Purpose:       Implementation of wxExVCS class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/extension/vcs.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>

wxExVCS* wxExVCS::m_Self = NULL;
#if wxUSE_GUI
wxExSTCEntryDialog* wxExVCS::m_STCEntryDialog = NULL;
#endif

wxExVCS::wxExVCS()
  : m_Command(VCS_NO_COMMAND)
  , m_FullPath(wxEmptyString)
{
  Initialize();
}

wxExVCS::wxExVCS(int command_id, const wxString& fullpath)
  : m_Command(GetType(command_id))
  , m_FullPath(fullpath)
{
  Initialize();
}

wxExVCS::wxExVCS(wxExVCSCommand type, const wxString& fullpath)
  : m_Command(type)
  , m_FullPath(fullpath)
{
  Initialize();
}

#if wxUSE_GUI
int wxExVCS::ConfigDialog(
  wxWindow* parent,
  const wxString& title) const
{
  std::vector<wxExConfigItem> v;
  v.push_back(wxExConfigItem()); // a spacer

  std::map<long, const wxString> choices;
  choices.insert(std::make_pair(VCS_NONE, _("None")));
  choices.insert(std::make_pair(VCS_GIT, "GIT"));
  choices.insert(std::make_pair(VCS_SVN, "SVN"));
  v.push_back(wxExConfigItem("VCS", choices));

  v.push_back(wxExConfigItem("GIT", CONFIG_FILEPICKERCTRL));
  v.push_back(wxExConfigItem("SVN", CONFIG_FILEPICKERCTRL));
  v.push_back(wxExConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL));

  return wxExConfigDialog(parent, v, title).ShowModal();
}
#endif

bool wxExVCS::DirExists(const wxFileName& filename) const
{
  if (!Use())
  {
    return false;
  }

  wxFileName path(filename);

  switch (wxConfigBase::Get()->ReadLong("VCS", VCS_SVN))
  {
    case VCS_GIT: path.AppendDir(".git"); break;
    case VCS_SVN: path.AppendDir(".svn"); break;
    default: wxFAIL;
  }

  return path.DirExists();
}

long wxExVCS::Execute()
{
  wxASSERT(m_Command != VCS_NO_COMMAND);

  const wxString cwd = 
    (m_Command == VCS_HELP ? wxEmptyString: wxGetCwd());

  wxString file;

  if (m_FullPath.empty() && !cwd.empty())
  {
    if (!wxSetWorkingDirectory(wxExConfigFirstOf(_("Base folder"))))
    {
      m_Output = _("Cannot set working directory");
      return -1;
    }

    if (m_Command == VCS_ADD)
    {
      file = " " + wxExConfigFirstOf(_("Path"));
    }
  }
  else if (!m_FullPath.empty())
  {
    file = " \"" + m_FullPath + "\"";
  }

  wxString comment;

  if (m_Command == VCS_COMMIT)
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

  m_CommandWithFlags = m_CommandString + flags;

  wxString vcs_bin;

  switch (wxConfigBase::Get()->ReadLong("VCS", VCS_SVN))
  {
    case VCS_GIT: vcs_bin = wxConfigBase::Get()->Read("GIT"); break;
    case VCS_SVN: vcs_bin = wxConfigBase::Get()->Read("SVN"); break;
    default: wxFAIL;
  }

  const wxString commandline = 
    vcs_bin + " " + m_CommandString + subcommand + flags + comment + file;

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

  if (m_FullPath.empty() && !cwd.empty())
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
wxStandardID wxExVCS::ExecuteDialog(wxWindow* parent)
{
  if (ShowDialog(parent) == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  if (UseFlags())
  {
    wxConfigBase::Get()->Write(m_FlagsKey, 
      wxConfigBase::Get()->Read(_("Flags")));
  }

  Execute();
  
  return wxID_OK;
}
#endif

wxExVCS* wxExVCS::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExVCS;

    if (!wxConfigBase::Get()->Exists("VCS"))
    {
      wxConfigBase::Get()->Write("VCS", (long)VCS_NONE);
    }
  }

  return m_Self;
}

wxExVCS::wxExVCSCommand wxExVCS::GetType(int command_id) const
{
  switch (command_id)
  {
    case ID_EDIT_VCS_ADD: 
    case ID_VCS_ADD:
      return VCS_ADD; break;

    case ID_EDIT_VCS_BLAME: 
    case ID_VCS_BLAME:
      return VCS_BLAME; break;

    case ID_EDIT_VCS_CAT: 
      return VCS_CAT; break;

    case ID_EDIT_VCS_COMMIT: 
    case ID_VCS_COMMIT:
      return VCS_COMMIT; break;

    case ID_EDIT_VCS_DIFF: 
    case ID_VCS_DIFF:
      return VCS_DIFF; break;

    case ID_EDIT_VCS_HELP: 
    case ID_VCS_HELP:
      return VCS_HELP; break;

    case ID_EDIT_VCS_INFO: 
    case ID_VCS_INFO:
      return VCS_INFO; break;

    case ID_EDIT_VCS_LOG: 
    case ID_VCS_LOG:
      return VCS_LOG; break;

    case ID_EDIT_VCS_LS: 
    case ID_VCS_LS:
      return VCS_LS; break;

    case ID_EDIT_VCS_PROPLIST: 
    case ID_VCS_PROPLIST:
      return VCS_PROPLIST; break;

    case ID_EDIT_VCS_PROPSET: 
    case ID_VCS_PROPSET:
      return VCS_PROPSET; break;

    case ID_EDIT_VCS_REVERT: 
    case ID_VCS_REVERT:
      return VCS_REVERT; break;

    case ID_EDIT_VCS_STAT: 
    case ID_VCS_STAT:
      return VCS_STAT; break;

    case ID_EDIT_VCS_UPDATE: 
    case ID_VCS_UPDATE:
      return VCS_UPDATE; break;

    default:
      wxFAIL;
      return VCS_NO_COMMAND;
      break;
  }
}

void wxExVCS::Initialize()
{
  if (Use() && m_Command != VCS_NO_COMMAND)
  {
    switch (wxConfigBase::Get()->ReadLong("VCS", VCS_SVN))
    {
      case VCS_GIT:
        switch (m_Command)
        {
          case VCS_ADD:      m_CommandString = "add"; break;
          case VCS_BLAME:    m_CommandString = "blame"; break;
          case VCS_CAT:      m_CommandString = "cat"; break;
          case VCS_COMMIT:   m_CommandString = "push"; break;
          case VCS_DIFF:     m_CommandString = "diff"; break;
          case VCS_HELP:     m_CommandString = "help"; break;
          case VCS_INFO:     m_CommandString = "info"; break;
          case VCS_LOG:      m_CommandString = "log"; break;
          case VCS_LS:       m_CommandString = "ls"; break;
          case VCS_PROPLIST: break;
          case VCS_PROPSET:  break;
          case VCS_REVERT:   m_CommandString = "revert"; break;
          case VCS_STAT:     m_CommandString = "status"; break;
          case VCS_UPDATE:   m_CommandString = "update"; break;
          default:
            wxFAIL;
            break;
        }
        break;

      case VCS_SVN:
        switch (m_Command)
        {
          case VCS_ADD:      m_CommandString = "add"; break;
          case VCS_BLAME:    m_CommandString = "blame"; break;
          case VCS_CAT:      m_CommandString = "cat"; break;
          case VCS_COMMIT:   m_CommandString = "commit"; break;
          case VCS_DIFF:     m_CommandString = "diff"; break;
          case VCS_HELP:     m_CommandString = "help"; break;
          case VCS_INFO:     m_CommandString = "info"; break;
          case VCS_LOG:      m_CommandString = "log"; break;
          case VCS_LS:       m_CommandString = "ls"; break;
          case VCS_PROPLIST: m_CommandString = "proplist"; break;
          case VCS_PROPSET:  m_CommandString = "propset"; break;
          case VCS_REVERT:   m_CommandString = "revert"; break;
          case VCS_STAT:     m_CommandString = "stat"; break;
          case VCS_UPDATE:   m_CommandString = "update"; break;
          default:
            wxFAIL;
            break;
        }
        break;

      default: wxFAIL;
    }

    m_Caption = "VCS " + m_CommandString;

    // Currently no flags, as no command was executed.
    m_CommandWithFlags = m_CommandString;

    // Use general key.
    m_FlagsKey = wxString::Format("cvsflags/name%d", m_Command);
  }

  m_Output.clear();
}

#if wxUSE_GUI
wxStandardID wxExVCS::Request(wxWindow* parent)
{
  wxStandardID retValue;

  if ((retValue = ExecuteDialog(parent)) == wxID_OK)
  {
    if (!m_Output.empty())
    {
      ShowOutput(parent);
    }
  }

  return retValue;
}
#endif

wxExVCS* wxExVCS::Set(wxExVCS* vcs)
{
  wxExVCS* old = m_Self;
  m_Self = vcs;
  return old;
}

#if wxUSE_GUI
int wxExVCS::ShowDialog(wxWindow* parent)
{
  std::vector<wxExConfigItem> v;

  if (m_Command == VCS_COMMIT)
  {
    v.push_back(wxExConfigItem(
      _("Revision comment"), 
      CONFIG_COMBOBOX,
      wxEmptyString,
      true)); // required
  }

  if (m_FullPath.empty() && m_Command != VCS_HELP)
  {
    v.push_back(wxExConfigItem(
      _("Base folder"), 
      CONFIG_COMBOBOXDIR, 
      wxEmptyString, 
      true)); // required

    if (m_Command == VCS_ADD)
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
      wxConfigBase::Get()->Read(m_FlagsKey));

    v.push_back(wxExConfigItem(_("Flags")));
  }

  if (UseSubcommand())
  {
    v.push_back(wxExConfigItem(_("Subcommand")));
  }

  return wxExConfigDialog(parent,
    v,
    m_Caption).ShowModal();
}
#endif

#if wxUSE_GUI
void wxExVCS::ShowOutput(wxWindow* parent) const
{
  wxString caption = m_Caption;
      
  if (m_Command != VCS_HELP)
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
    (m_Command == VCS_CAT || m_Command == VCS_BLAME))
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

bool wxExVCS::Use() const
{
  return wxConfigBase::Get()->ReadLong("VCS", VCS_NONE) != VCS_NONE;
}

bool wxExVCS::UseFlags() const
{
  return m_Command != VCS_UPDATE && m_Command != VCS_HELP;
}

bool wxExVCS::UseSubcommand() const
{
  return m_Command == VCS_HELP;
}
