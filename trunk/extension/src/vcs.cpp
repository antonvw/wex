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
#include <wx/extension/vcs.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>

/// VCS types supported.
/// See also defs.h, and do not exceed VCS_MAX_COMMANDS.
enum wxExCommand
{
    VCS_NO_COMMAND, ///< not ok value
    VCS_ADD,      ///< vcs add
    VCS_BLAME,    ///< vcs blame
    VCS_CAT,      ///< vcs cat
    VCS_COMMIT,   ///< vcs commit
    VCS_DIFF,     ///< vcs diff
    VCS_HELP,     ///< vcs help
    VCS_INFO,     ///< vcs info
    VCS_LOG,      ///< vcs log
    VCS_LS,       ///< vcs ls
    VCS_PROPLIST, ///< vcs prop list
    VCS_PROPSET,  ///< vcs prop set
    VCS_PUSH,     ///< vcs push
    VCS_REVERT,   ///< vcs revert
    VCS_SHOW,     ///< vcs show
    VCS_STAT,     ///< vcs stat
    VCS_UPDATE,   ///< vcs update
};

enum wxExSystem
{
  VCS_NONE, ///< no version control
  VCS_AUTO, ///< Uses the VCS appropriate for current file
};

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

#if wxUSE_GUI
void wxExVCS::BuildMenu(int base_id, wxMenu* menu, bool is_popup)
{
  for (
    auto it = m_Entries.begin();
    it != m_Entries.end();
    ++it)
  {
    const int id = base_id + it->second.GetNo();
    const wxString text(wxExEllipsed("&" + it->second.GetName()));
    menu->Append(id, text);
  }
}
#endif

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
  choices.insert(std::make_pair(VCS_AUTO, "Auto"));
  
  for (
    auto it = m_Entries.begin();
    it != m_Entries.end();
    ++it)
  {
    choices.insert(std::make_pair(it->second.GetNo(), it->second.GetName());
  }

  v.push_back(wxExConfigItem("VCS", choices));

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
  if (Use(filename))
  {
    if (CheckSVN(filename))
    {
      return true;
    }
    else if (CheckGIT(filename))
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

    if (m_Command == VCS_ADD)
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

  const wxString vcs_bin = wxConfigBase::Get()->Read(GetName(), "svn");

  if (vcs_bin.empty())
  {
    wxLogError(GetName() + " " + _("path is empty"));
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
    m_Self->Read();

    // Add default VCS.
    if (!wxConfigBase::Get()->Exists("VCS") && !m_Entries.empty())
    {
      // TODO: Add SVN only if svn bin exists on linux.
      wxConfigBase::Get()->Write("VCS", (long)VCS_AUTO + 1);
    }
  }

  return m_Self;
}

const wxString wxExVCS::GetName() const
{
  if (Use())
  {
    for (
      auto it = m_Entries.begin();
      it != m_Entries.end();
      ++it)
    {
      if (it->second.GetNo() == vcs)
      {
        return it->second.GetName();
      }
    }
  }

  return wxEmptyString;
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

void wxExVCS::Initialize()
{
  if (Use() && m_Command != VCS_NO_COMMAND)
  {
    m_CommandString = m_Entries[GetName()].GetCommand(m_Command);
    m_Caption = GetName() + " " + m_CommandString;

    // Currently no flags, as no command was executed.
    m_CommandWithFlags = m_CommandString;

    // Use general key.
    m_FlagsKey = wxString::Format("vcsflags/name%d", m_Command);
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
  return GetName() == "svn";
}

bool wxExVCS::Use() const
{
  return Use(m_FileName);
}

bool wxExVCS::Use(const wxFileName& filename) const
{
  long vcs = wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO + 1);

  if (vcs == VCS_AUTO)
  {
    if (CheckSVN(filename))
    {
      return true;
    }
    else if (CheckGIT(filename))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  return vcs != VCS_NONE;
}

bool wxExVCS::UseFlags() const
{
  return m_Command != VCS_UPDATE && m_Command != VCS_HELP;
}

bool wxExVCS::UseSubcommand() const
{
  return m_Command == VCS_HELP;
}

int wxExVCSEntry::m_Instances = VCS_AUTO + 1;

wxExVCSEntry::wxExVCSEntry()
{
}

wxExVCSEntry::wxExVCSEntry(const wxXmlNode* node)
  : m_No(m_Instances++)
{
  m_Name = node->GetAttribute("name");

  if (m_Name.empty())
  {
    wxLogError(_("Missing vcs on line: %d"), node->GetLineNumber());
  }
  else
  {
    wxXmlNode *child = node->GetChildren();

    while (child)
    {
      if (child->GetName() == "commands")
      {
        m_Commands = ParseNodeCommands(child);
      }
      else if (child->GetName() == "comment")
      {
        // Ignore comments.
      }
      else
      {
        wxLogError(_("Undefined tag: %s on line: %d"),
          child->GetName().c_str(),
          child->GetLineNumber());
      }

      child = child->GetNext();
    }
  }
}

const wxString wxExVCSEntry::GetCommand(int command_id) const
{
  const auto it = m_Commands.find(command_id);
  return (it != m_Commands.end() ? it->second.c_str(): wxEmptyString);
}
  
const std::map<int, wxString> wxExVCSEntry::ParseNodeCommands(
  const wxXmlNode* node) const
{
  std::map<int, wxString> text;

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "command")
    {
      const int no = node->GetAttribute("no", "0");
      text.insert(std::make_pair(no, child->GetName()));
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined tag: %s on line: %d"),
        child->GetName().c_str(),
        child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return text;
}
