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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
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
  , m_FileName(wxExFileName())
{
  Initialize();
}

wxExVCS::wxExVCS(int command_id, const wxExFileName& filename)
  : m_Command(GetType(command_id))
  , m_FileName(filename)
{
  Initialize();
}

wxExVCS::wxExVCS(wxExCommand type, const wxExFileName& filename)
  : m_Command(type)
  , m_FileName(filename)
{
  Initialize();
}

bool wxExVCS::CheckGIT(const wxFileName& fn) const
{
  // The .git dir only exists in the root, so check all components.
  wxFileName root(fn.GetPath());

  while (root.DirExists() && root.GetDirCount() > 0)
  {
    wxFileName path(root);
    path.AppendDir(".git");

    if (path.DirExists())
    {
      return true;
    }

    root.RemoveLastDir();
  }

  return false;
}

bool wxExVCS::CheckSVN(const wxFileName& fn) const
{
  // these cannot be combined, as AppendDir is a void (2.9.1).
  wxFileName path(fn);
  path.AppendDir(".svn");
  return path.DirExists();
}

#if wxUSE_GUI
int wxExVCS::ConfigDialog(
  wxWindow* parent,
  const wxString& title) const
{
  std::vector<wxExConfigItem> v;

  std::map<long, const wxString> choices;
  choices.insert(std::make_pair(VCS_NONE, _("None")));
  choices.insert(std::make_pair(VCS_GIT, "GIT"));
  choices.insert(std::make_pair(VCS_SVN, "SVN"));
  choices.insert(std::make_pair(VCS_AUTO, "Auto"));
  v.push_back(wxExConfigItem("VCS", choices));

  v.push_back(wxExConfigItem("GIT", CONFIG_FILEPICKERCTRL));
  v.push_back(wxExConfigItem("SVN", CONFIG_FILEPICKERCTRL));
  v.push_back(wxExConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL));

  return wxExConfigDialog(parent, v, title).ShowModal();
}
#endif

bool wxExVCS::DirExists(const wxFileName& filename) const
{
  switch (GetVCS(filename))
  {
    case VCS_GIT: 
      return CheckGIT(filename); break;

    case VCS_NONE: break; // prevent wxFAIL
    
    case VCS_SVN: 
      return CheckSVN(filename); break;

    default: wxFAIL;
  }

  return false;
}

long wxExVCS::Execute()
{
  wxASSERT(m_Command != VCS_NO_COMMAND);

  wxString cwd;
  wxString file;

  if (!m_FileName.IsOk())
  {
    cwd = wxGetCwd();
    wxSetWorkingDirectory(wxExConfigFirstOf(_("Base folder")));

    if (m_Command == VCS_ADD)
    {
      file = " " + wxExConfigFirstOf(_("Path"));
    }
  }
  else
  {
    if (GetVCS() == VCS_GIT)
    {
      cwd = wxGetCwd();
      wxSetWorkingDirectory(m_FileName.GetPath());
      file = " \"" + m_FileName.GetFullName() + "\"";
    }
    else
    {
      file = " \"" + m_FileName.GetFullPath() + "\"";
    }
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

      // If we specified help flags, we do not need a file argument.      
      if (flags.Contains("help"))
      {
        file.clear();
      }
    }
  }

  m_CommandWithFlags = m_CommandString + flags;

  wxString vcs_bin;

  switch (GetVCS())
  {
    case VCS_GIT: vcs_bin = wxConfigBase::Get()->Read("GIT", "git"); break;
    case VCS_SVN: vcs_bin = wxConfigBase::Get()->Read("SVN", "svn"); break;
    default: wxFAIL;
  }

  if (vcs_bin.empty())
  {
    wxLogError(GetVCSName() + " " + _("path is empty"));
    return -1;
  }

  const wxString commandline = 
    vcs_bin + " " + m_CommandString + subcommand + flags + comment + file;

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(commandline);
#endif

  wxArrayString output;
  wxArrayString errors;
  long retValue;

  // Call wxExcute to execute the cvs command and
  // collect the output and the errors.
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

  if (!cwd.empty())
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

  const auto retValue = Execute();
  
  return (retValue != -1 ? wxID_OK: wxID_CANCEL);
}
#endif

wxExVCS* wxExVCS::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExVCS;

    // Add default VCS.
    if (!wxConfigBase::Get()->Exists("VCS"))
    {
      // TODO: Add SVN only if svn bin exists on linux.
      wxConfigBase::Get()->Write("VCS", (long)VCS_SVN);
    }
  }

  return m_Self;
}

wxExVCS::wxExCommand wxExVCS::GetType(int command_id) const
{
  if (command_id > ID_VCS_LOWEST && command_id < ID_VCS_HIGHEST)
  {
    return (wxExCommand)(command_id - ID_VCS_LOWEST);
  }
  else if (command_id > ID_EDIT_VCS_LOWEST && command_id < ID_EDIT_VCS_HIGHEST)
  {
    return (wxExCommand)(command_id - ID_EDIT_VCS_LOWEST);
  }
  else
  {
    wxFAIL;
    return VCS_NO_COMMAND;
  }
}

long wxExVCS::GetVCS(const wxFileName& filename) const
{
  const wxExFileName fn = (!filename.IsOk() ? m_FileName: filename);
  
  long vcs = wxConfigBase::Get()->ReadLong("VCS", VCS_SVN);

  if (vcs == VCS_AUTO)
  {
    if (CheckSVN(fn))
    {
      vcs = VCS_SVN;
    }
    else if (CheckGIT(fn))
    {
      vcs = VCS_GIT;
    }
    else
    {
      vcs = VCS_NONE;
    }
  }

  return vcs;
}

const wxString wxExVCS::GetVCSName() const
{
  wxString text;

  switch (GetVCS())
  {
    case VCS_GIT: text = "GIT"; break;
    case VCS_SVN: text = "SVN"; break;
    default: wxFAIL;
  }

  return text;
}

void wxExVCS::Initialize()
{
  if (Use() && m_Command != VCS_NO_COMMAND)
  {
    switch (GetVCS())
    {
      case VCS_GIT:
        switch (m_Command)
        {
          case VCS_ADD:      m_CommandString = "add"; break;
          case VCS_BLAME:    m_CommandString = "blame"; break;
          case VCS_CAT:      break;
          case VCS_COMMIT:   m_CommandString = "commit"; break;
          case VCS_DIFF:     m_CommandString = "diff"; break;
          case VCS_HELP:     m_CommandString = "help"; break;
          case VCS_INFO:     break;
          case VCS_LOG:      m_CommandString = "log"; break;
          case VCS_LS:       break;
          case VCS_PROPLIST: break;
          case VCS_PROPSET:  break;
          case VCS_PUSH:     m_CommandString = "push"; break;
          case VCS_REVERT:   m_CommandString = "revert"; break;
          case VCS_SHOW:     m_CommandString = "show"; break;
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
          case VCS_PUSH:     break;
          case VCS_REVERT:   m_CommandString = "revert"; break;
          case VCS_SHOW:     break;
          case VCS_STAT:     m_CommandString = "stat"; break;
          case VCS_UPDATE:   m_CommandString = "update"; break;
          default:
            wxFAIL;
            break;
        }
        break;

      default: wxFAIL;
    }

    m_Caption = GetVCSName() + " " + m_CommandString;

    // Currently no flags, as no command was executed.
    m_CommandWithFlags = m_CommandString;

    // Use general key.
    m_FlagsKey = wxString::Format("cvsflags/name%d", m_Command);
  }

  m_Output.clear();
}

bool wxExVCS::IsOpenCommand() const
{
  return 
    m_Command == wxExVCS::VCS_BLAME ||
    m_Command == wxExVCS::VCS_CAT ||
    m_Command == wxExVCS::VCS_DIFF;
}

#if wxUSE_GUI
wxStandardID wxExVCS::Request(wxWindow* parent)
{
  wxStandardID retValue;

  if ((retValue = ExecuteDialog(parent)) == wxID_OK)
  {
    ShowOutput(parent);
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

  if (!m_FileName.IsOk() && m_Command != VCS_HELP)
  {
    v.push_back(wxExConfigItem(
      _("Base folder"), 
      CONFIG_COMBOBOXDIR, 
      wxEmptyString, 
      true,
      1000));

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
    caption += " " + (m_FileName.IsOk() ?  
      m_FileName.GetFullName(): 
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
      wxDefaultPosition,
      wxSize(350, 50));
  }
  else
  {
    m_STCEntryDialog->SetText(m_Output);
    m_STCEntryDialog->SetTitle(caption);
  }

  // Add a lexer when appropriate.
  if (m_Command == VCS_CAT || m_Command == VCS_BLAME)
  {
    if (m_FileName.GetLexer().IsOk())
    {
      m_STCEntryDialog->SetLexer(m_FileName.GetLexer().GetScintillaLexer());
    }
  }
  else if (m_Command == VCS_DIFF)
  {
    m_STCEntryDialog->SetLexer("diff");
  }
  else
  {
    m_STCEntryDialog->SetLexer(wxEmptyString);
  }

  m_STCEntryDialog->Show();
}
#endif

bool wxExVCS::SupportKeywordExpansion() const
{
  return GetVCS() == VCS_SVN;
}
bool wxExVCS::Use() const
{
  return GetVCS() != VCS_NONE;
}

bool wxExVCS::UseFlags() const
{
  return m_Command != VCS_UPDATE && m_Command != VCS_HELP;
}

bool wxExVCS::UseSubcommand() const
{
  return m_Command == VCS_HELP;
}
