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
  VCS_NO_COMMAND,
};

// The VCS systems are read in from vcss.xml.
// The ones here can be set in the config dialog, and are not
// present in the vcss.xml.
enum
{
  VCS_NONE, // no version control
  VCS_AUTO, // uses the VCS appropriate for current file
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

#if wxUSE_GUI
void wxExVCS::BuildMenu(int base_id, wxMenu* menu, bool is_popup)
{
  m_Entries[GetName()].BuildMenu(base_id, menu, is_popup);
}
#endif

bool wxExVCS::CheckPath(const wxString& vcs, const wxFileName& fn) const
{
  // these cannot be combined, as AppendDir is a void (2.9.1).
  wxFileName path(fn);
  path.AppendDir("." + vcs);
  return path.DirExists();
}

bool wxExVCS::CheckPathAll(const wxString& vcs, const wxFileName& fn) const
{
  // The .git dir only exists in the root, so check all components.
  wxFileName root(fn.GetPath());

  while (root.DirExists() && root.GetDirCount() > 0)
  {
    wxFileName path(root);
    path.AppendDir("." + vcs);

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
    if (CheckPath("svn", filename))
    {
      return true;
    }
    else if (CheckPathAll("git", filename))
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

    if (m_CommandString == "add")
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

  if (m_CommandString == "commit")
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
    // This is a static method, so not use m_Entries.
    if (!wxConfigBase::Get()->Exists("VCS"))
    {
      // TODO: Add SVN only if svn bin exists on linux.
      wxConfigBase::Get()->Write("VCS", (long)VCS_AUTO + 1);
    }
  }

  return m_Self;
}

const wxString wxExVCS::GetName() const
{
  const long no = Use();
  
  if (no != VCS_NONE)
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
  }

  return wxEmptyString;
}

long wxExVCS::GetNo(const wxString& name) const
{
  for (
    auto it = m_Entries.begin();
    it != m_Entries.end();
    ++it)
  {
    if (it->second.GetName() == name)
    {
      return it->second.GetNo();
    }
  }

  return VCS_NONE;
}

int wxExVCS::GetType(int command_id) const
{
  if (command_id > ID_VCS_LOWEST && command_id < ID_VCS_HIGHEST)
  {
    return command_id - ID_VCS_LOWEST;
  }
  else if (command_id > ID_EDIT_VCS_LOWEST && command_id < ID_EDIT_VCS_HIGHEST)
  {
    return command_id - ID_EDIT_VCS_LOWEST;
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
    m_CommandString == "blame" ||
    m_CommandString == "cat" ||
    m_CommandString == "diff";
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

  if (m_CommandString == "commit")
  {
    v.push_back(wxExConfigItem(
      _("Revision comment"), 
      CONFIG_COMBOBOX,
      wxEmptyString,
      true)); // required
  }

  if (!m_FileName.IsOk() && m_CommandString != "help")
  {
    v.push_back(wxExConfigItem(
      _("Base folder"), 
      CONFIG_COMBOBOXDIR, 
      wxEmptyString, 
      true,
      1000));

    if (m_CommandString == "add")
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
      
  if (m_CommandString != "help")
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
  if (m_CommandString == "cat" || m_CommandString == "blame")
  {
    if (m_FileName.GetLexer().IsOk())
    {
      m_STCEntryDialog->SetLexer(m_FileName.GetLexer().GetScintillaLexer());
    }
  }
  else if (m_CommandString == "diff")
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

long wxExVCS::Use(const wxFileName& filename) const
{
  long vcs = wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO + 1);

  if (vcs == VCS_AUTO)
  {
    if (CheckPath("svn", filename))
    {
      return GetNo("svn");
    }
    else if (CheckPathAll("git", filename))
    {
      return GetNo("git");
    }
    else
    {
      return VCS_NONE;
    }
  }

  return vcs;
}

bool wxExVCS::UseFlags() const
{
  return m_CommandString != "update" && m_CommandString != "help";
}

bool wxExVCS::UseSubcommand() const
{
  return m_CommandString == "help";
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

#if wxUSE_GUI
void wxExVCSEntry::BuildMenu(int base_id, wxMenu* menu, bool is_popup)
{
  for (
    auto it = m_Commands.begin();
    it != m_Commands.end();
    ++it)
  {
    const int id = base_id + m_No;
    const wxString text(wxExEllipsed("&" + *it));
    menu->Append(id, text);
  }
}
#endif

const wxString wxExVCSEntry::GetCommand(int command_id) const
{
  return m_Commands.at(command_id);
}
  
const std::vector<wxString> wxExVCSEntry::ParseNodeCommands(
  const wxXmlNode* node) const
{
  std::vector<wxString> text;

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "command")
    {
      const wxString content = child->GetNodeContent().Strip(wxString::both);
      text.push_back(content);
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
