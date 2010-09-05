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
#include <wx/menu.h>
#include <wx/stdpaths.h>
#include <wx/extension/vcs.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>

// The VCS commands are read in from vcss.xml.
// See also defs.h, and do not exceed VCS_MAX_COMMANDS in vcss.xml.
// This command indicates an error in the command being handled.
enum
{
  VCS_NO_COMMAND = -1,
};

// The VCS systems are read in from vcss.xml.
// The ones here can be set in the config dialog, and are not
// present in the vcss.xml.
enum
{
  VCS_NONE = 0, // no version control
  VCS_AUTO, // uses the VCS appropriate for current file
};

int wxExVCS::m_Command = VCS_NO_COMMAND;
std::map<wxString, wxExVCSEntry> wxExVCS::m_Entries;
wxExFileName wxExVCS::m_FileName;
wxExVCS* wxExVCS::m_Self = NULL;
#if wxUSE_GUI
wxExSTCEntryDialog* wxExVCS::m_STCEntryDialog = NULL;
#endif

wxExVCS::wxExVCS()
{
  Initialize();
}

wxExVCS::wxExVCS(int command_id, const wxExFileName& filename)
{
  m_Command = GetType(command_id);
  m_FileName = filename;
  
  Initialize();
}

#if wxUSE_GUI
void wxExVCS::BuildMenu(
  int base_id, 
  wxMenu* menu, 
  const wxExFileName& filename,
  bool is_popup)
{
  if (filename.IsOk())
  {
    m_FileName = filename;
  }
  
  const auto it = m_Entries.find(GetName());
    
  if (it != m_Entries.end())
  {
      it->second.BuildMenu(base_id, menu, is_popup);
  }
}
#endif

bool wxExVCS::CheckPath(const wxString& vcs, const wxFileName& fn)
{
  // these cannot be combined, as AppendDir is a void (2.9.1).
  wxFileName path(fn);
  path.AppendDir("." + vcs);
  return path.DirExists();
}

bool wxExVCS::CheckPathAll(
  const wxString& vcs, 
  const wxFileName& fn)
{
  const wxString use_vcs = (vcs == "mercurial" ? "hg": vcs);
  
  // The .git dir only exists in the root, so check all components.
  wxFileName root(fn.GetPath());

  while (root.DirExists() && root.GetDirCount() > 0)
  {
    wxFileName path(root);
    path.AppendDir("." + use_vcs);

    if (path.DirExists())
    {
      return true;
    }

    root.RemoveLastDir();
  }

  return false;
}

#if wxUSE_GUI
int wxExVCS::ConfigDialog(
  wxWindow* parent,
  const wxString& title) const
{
  std::vector<wxExConfigItem> v;

  std::map<long, const wxString> choices;
  choices.insert(std::make_pair((long)VCS_NONE, _("None")));
  choices.insert(std::make_pair((long)VCS_AUTO, "Auto"));
  
  for (
    auto it = m_Entries.begin();
    it != m_Entries.end();
    ++it)
  {
    choices.insert(std::make_pair(it->second.GetNo(), it->second.GetName()));
  }

  v.push_back(wxExConfigItem(
    "VCS",
    choices,
    true, // use a radiobox 
    wxEmptyString, 
    5));  // no more than 5 cols

  for (
    auto it2 = m_Entries.begin();
    it2 != m_Entries.end();
    ++it2)
  {
    v.push_back(wxExConfigItem(it2->second.GetName(), CONFIG_FILEPICKERCTRL));
  }

  v.push_back(wxExConfigItem(_("Comparator"), CONFIG_FILEPICKERCTRL));

  return wxExConfigDialog(parent, v, title).ShowModal();
}
#endif

bool wxExVCS::DirExists(const wxFileName& filename) const
{
  if (Use(filename) != VCS_NONE)
  {
    // When adding a vcs, also check Use.
    if (CheckPath("svn", filename))
    {
      return true;
    }
    else if (CheckPathAll("git", filename))
    {
      return true;
    }
    else if (CheckPathAll("mercurial", filename))
    {
      return true;
    }
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

    if (m_CommandString.IsAdd())
    {
      file = " " + wxExConfigFirstOf(_("Path"));
    }
  }
  else
  {
    if (GetName() == "git")
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

  if (m_CommandString.IsCommit())
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

  m_CommandWithFlags = m_CommandString.GetCommand() + flags;

  const wxString vcs_bin = wxConfigBase::Get()->Read(GetName(), "svn");

  if (vcs_bin.empty())
  {
    wxLogError(GetName() + " " + _("path is empty"));
    return -1;
  }

  const wxString commandline = 
    vcs_bin + " " + 
    m_CommandString.GetCommand() + subcommand + flags + comment + file;

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
    m_Self->Read();

    // Add default VCS.
    // This is a static method, so not use m_Entries.
    if (!wxConfigBase::Get()->Exists("VCS"))
    {
      wxConfigBase::Get()->Write("VCS", (long)VCS_AUTO + 1);
    }
  }

  return m_Self;
}

const wxString wxExVCS::GetName()
{
  const long no = Use(m_FileName);
  
  if (no != VCS_NONE && no != VCS_AUTO)
  {
    for (
      auto it = m_Entries.begin();
      it != m_Entries.end();
      ++it)
    {
      if (it->second.GetNo() == no)
      {
        return it->second.GetName();
      }
    }
    
    wxFAIL;
  }

  return wxEmptyString;
}

long wxExVCS::GetNo(const wxString& name)
{
  const auto it = m_Entries.find(name);
  
  if (it != m_Entries.end())
  {
    return it->second.GetNo();
  }
  else
  {
    wxFAIL;
    return VCS_NONE;
  }
}

int wxExVCS::GetType(int command_id) const
{
  if (command_id > ID_VCS_LOWEST && command_id < ID_VCS_HIGHEST)
  {
    return command_id - ID_VCS_LOWEST - 1;
  }
  else if (command_id > ID_EDIT_VCS_LOWEST && command_id < ID_EDIT_VCS_HIGHEST)
  {
    return command_id - ID_EDIT_VCS_LOWEST - 1;
  }
  else
  {
    wxFAIL;
    return VCS_NO_COMMAND;
  }
}

void wxExVCS::Initialize()
{
  if (Use() && m_Command != VCS_NO_COMMAND)
  {
    const auto it = m_Entries.find(GetName());
  
    if (it != m_Entries.end())
    {
      m_CommandString.SetCommand(it->second.GetCommand(m_Command));
      m_Caption = GetName() + " " + m_CommandString.GetCommand();

      // Currently no flags, as no command was executed.
      m_CommandWithFlags = m_CommandString.GetCommand();

      // Use general key.
      m_FlagsKey = wxString::Format("vcsflags/name%d", m_Command);
    }
    else
    {
      wxFAIL;
    }
  }

  m_Output.clear();
}

bool wxExVCS::Read()
{
  const wxFileName filename(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + "vcss.xml");

  // This test is to prevent showing an error if the vcs file does not exist,
  // as this is not required.
  if (!filename.FileExists())
  {
    return false;
  }

  wxXmlDocument doc;

  if (!doc.Load(filename.GetFullPath()))
  {
    return false;
  }

  // Initialize members.
  m_Entries.clear();

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
    if (child->GetName() == "vcs")
    {
      const wxExVCSEntry vcs(child);
      m_Entries.insert(std::make_pair(vcs.GetName(), vcs));
    }

    child = child->GetNext();
  }

  return true;
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

  if (m_CommandString.IsCommit())
  {
    v.push_back(wxExConfigItem(
      _("Revision comment"), 
      CONFIG_COMBOBOX,
      wxEmptyString,
      true)); // required
  }

  if (!m_FileName.IsOk() && !m_CommandString.IsHelp())
  {
    v.push_back(wxExConfigItem(
      _("Base folder"), 
      CONFIG_COMBOBOXDIR, 
      wxEmptyString, 
      true,
      1000));

    if (m_CommandString.IsAdd())
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
  
  if (v.empty())
  {
    return wxMessageBox(m_Caption);
  }
  else
  {
    return wxExConfigDialog(parent,
      v,
      m_Caption).ShowModal();
  }
}
#endif

#if wxUSE_GUI
void wxExVCS::ShowOutput(wxWindow* parent) const
{
  wxString caption = m_Caption;
      
  if (!m_CommandString.IsHelp())
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
  if (m_CommandString.IsOpen())
  {
    if (m_FileName.GetLexer().IsOk())
    {
      m_STCEntryDialog->SetLexer(m_FileName.GetLexer().GetScintillaLexer());
    }
  }
  else if (m_CommandString.IsDiff())
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
  const auto it = m_Entries.find(GetName());
    
  if (it != m_Entries.end())
  {
      return it->second.SupportKeywordExpansion();
  }
  
  return false;
}

bool wxExVCS::Use() const
{
  return Use(m_FileName) != VCS_NONE;
}

long wxExVCS::Use(const wxFileName& filename)
{
  const long vcs = wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO + 1);

  if (vcs == VCS_AUTO)
  {
    if (!filename.IsOk())
    {
      // We do not have a filename, so return AUTO.
      return vcs;
    }
    // When adding a vcs, also check DirExists.
    else if (CheckPath("svn", filename))
    {
      return GetNo("svn");
    }
    else if (CheckPathAll("git", filename))
    {
      return GetNo("git");
    }
    else if (CheckPathAll("mercurial", filename))
    {
      return GetNo("mercurial");
    }
    else
    {
      // We do not yet know vcs, so return AUTO.
      return vcs;
    }
  }

  return vcs;
}

bool wxExVCS::UseFlags() const
{
  return !m_CommandString.IsHelp();
}

bool wxExVCS::UseSubcommand() const
{
  return m_CommandString.IsHelp();
}
