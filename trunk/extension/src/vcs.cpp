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
#include <wx/extension/filename.h>
#include <wx/extension/util.h>

std::map<wxString, wxExVCSEntry> wxExVCS::m_Entries;
wxArrayString wxExVCS::m_Files;
wxFileName wxExVCS::m_FileName;
wxExVCS* wxExVCS::m_Self = NULL;

wxExVCS::wxExVCS(const wxFileName& filename)
{
  m_FileName = filename;
}

wxExVCS::wxExVCS(int menu_id, const wxString& file)
{
  if (wxFileName(file).IsOk())
  {
    m_Files.Clear();
    m_Files.Add(file);
  }
  
  Initialize(menu_id);
}

wxExVCS::wxExVCS(int menu_id, const wxArrayString& files)
{
  m_Files = files;
  
  Initialize(menu_id);
}

#if wxUSE_GUI
int wxExVCS::BuildMenu(
  int base_id, 
  wxMenu* menu, 
  const wxFileName& filename,
  bool is_popup)
{
  if (filename.IsOk())
  {
    m_Files.Clear();
    m_Files.Add(filename.GetFullPath());
  }
  
  return FindVCSEntry((!m_Files.empty() ? 
    m_Files[0]: wxString(wxEmptyString))).BuildMenu(base_id, menu, is_popup);
}
#endif

bool wxExVCS::CheckPath(const wxString& vcs, const wxFileName& fn) const
{
  if (!fn.IsOk() || vcs.empty())
  {
    return false;
  }
  
  // these cannot be combined, as AppendDir is a void (2.9.1).
  wxFileName path(fn);
  path.AppendDir(vcs);

  if (path.DirExists())
  {
    return true;
  }
  else
  {
    wxFileName path(fn);
    path.AppendDir("." + vcs);
    return path.DirExists();
  }
}

bool wxExVCS::CheckPathAll(
  const wxString& vcs, 
  const wxFileName& fn) const
{
  if (!fn.IsOk() || vcs.empty())
  {
    return false;
  }
  
  const wxString use_vcs = (vcs == "mercurial" ? "hg": vcs);
  
  // The .git dir only exists in the root, so check all components.
  wxFileName root(fn);

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
  if (m_Entries.empty())
  {
    return wxID_CANCEL;
  }
  
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

  // Estimate number of columns used by the radiobox.
  int cols = 5;

  switch (choices.size())
  {
    case 6:
    case 11:
      cols = 3;
      break;

    case 7:
    case 8:
    case 12:
    case 16:
      cols = 4;
      break;
  }

  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(
    "VCS",
    choices,
    true, // use a radiobox 
    wxEmptyString, 
    cols));

  for (
    auto it2 = m_Entries.begin();
    it2 != m_Entries.end();
    ++it2)
  {
    v.push_back(wxExConfigItem(it2->second.GetName(), CONFIG_FILEPICKERCTRL));
  }

  return wxExConfigDialog(parent, v, title).ShowModal();
}
#endif

bool wxExVCS::DirExists(const wxFileName& filename) const
{
  const wxString vcs = FindVCSEntry(filename).GetName();

  // When adding a vcs, also check GetNo.
  if ((vcs == "git" || vcs == "mercurial") && CheckPathAll(vcs, filename))
  {
    return true;
  }
  // This is the default check.
  else 
  {
    return CheckPath(vcs, filename);
  }
}

long wxExVCS::Execute()
{
  wxASSERT(!m_Files.empty());

  wxString wd;
  wxString file;
  
  const wxExVCSEntry vcs = FindVCSEntry(m_Files[0]);
  const wxString name(vcs.GetName());
  const wxFileName filename(m_Files[0]);

  if (!filename.IsOk())
  {
    wd = wxExConfigFirstOf(_("Base folder"));

    if (m_Command.IsAdd())
    {
      file = wxExConfigFirstOf(_("Path"));
    }
  }
  else
  {
    if (m_Files.size() > 1)
    {
      for (
        auto it = m_Files.begin();
        it != m_Files.end();
        it++)
      {
        file += *it + " ";
      }
    }
    else if (name == "git")
    {
      wd = filename.GetPath();
      file = "\"" + filename.GetFullName() + "\"";
    }
    else
    {
      file = "\"" + filename.GetFullPath() + "\"";
    }
  }

  wxString comment;

  if (m_Command.IsCommit())
  {
    comment = 
      "-m \"" + wxExConfigFirstOf(_("Revision comment")) + "\" ";
  }

  wxString subcommand;
  
  if (UseSubcommand())
  {
    subcommand = wxConfigBase::Get()->Read(_("Subcommand"));

    if (!subcommand.empty())
    {
      subcommand += " ";
    }
  }

  wxString flags;

  if (UseFlags())
  {
    flags = wxConfigBase::Get()->Read(_("Flags"));

    if (!flags.empty())
    {
      flags += " ";

      // If we specified help flags, we do not need a file argument.      
      if (flags.Contains("help"))
      {
        file.clear();
      }
    }
  }

  m_CommandWithFlags = m_Command.GetCommand() + flags;

  const wxString bin = wxConfigBase::Get()->Read(name, "svn");

  if (bin.empty())
  {
    wxLogError(name + " " + _("path is empty"));
    return -1;
  }

  if (vcs.GetFlagsLocation() == wxExVCSEntry::VCS_FLAGS_LOCATION_POSTFIX)
  {
    return wxExCommand::Execute(
      bin + " " + m_Command.GetCommand() + subcommand + flags + comment + file, 
      wd);
  }
  else
  {
    return wxExCommand::Execute(
      bin + " " + flags + m_Command.GetCommand() + subcommand + comment + file, 
      wd);
  }
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

  return (Execute() < 0 ? wxID_CANCEL: wxID_OK);
}
#endif

const wxExVCSEntry wxExVCS::FindVCSEntry(const wxFileName& filename) const
{
  const long vcs = wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO);

  if (vcs == VCS_AUTO)
  {
    if (!filename.IsOk())
    {
      return wxExVCSEntry();
    }
    else
    {
      for (
        auto it = m_Entries.begin();
        it != m_Entries.end();
        ++it)
      {
        const wxString name = it->second.GetName();

        if ((name == "git" || name == "mercurial") &&
             CheckPathAll(name, filename))
        {
          return it->second;
        }
        else if (CheckPath(name, filename))
        {
          return it->second;
        }
      }

      return wxExVCSEntry();
    }
  }
  else
  {
    for (
      auto it = m_Entries.begin();
      it != m_Entries.end();
      ++it)
    {
      if (it->second.GetNo() == vcs)
      {
        return it->second;
      }
    }
  }
}

wxExVCS* wxExVCS::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExVCS(wxFileName(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + "vcs.xml")
      );

    if (m_Self->Read())
    {
      // Add default VCS.
      // This is a static method, so not use m_Entries.
      if (!wxConfigBase::Get()->Exists("VCS"))
      {
        wxConfigBase::Get()->Write("VCS", (long)VCS_AUTO);
      }
    }
  }

  return m_Self;
}

void wxExVCS::Initialize(int menu_id)
{
  const wxExVCSEntry vcs = FindVCSEntry(
    !m_Files.empty() ? m_Files[0]: wxString(wxEmptyString));
  
  m_Command = vcs.GetCommand(menu_id);
  m_Caption = vcs.GetName() + " " + m_Command.GetCommand();
  m_FlagsKey = wxString::Format("vcsflags/name%d", m_Command.GetNo());
}

bool wxExVCS::Read()
{
  // This test is to prevent showing an error if the vcs file does not exist,
  // as this is not required.
  if (!m_FileName.FileExists())
  {
    return false;
  }

  wxXmlDocument doc;

  if (!doc.Load(m_FileName.GetFullPath()))
  {
    return false;
  }

  // Initialize members.
  const int old_entries = m_Entries.size();
  
  m_Entries.clear();
  wxExVCSEntry::ResetInstances();

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

  // If current number of entries differs from old one,
  // we added or removed an entry. That might give problems
  // with the vcs id stored in the config, so reset it. 
  if (old_entries != m_Entries.size())
  {
    wxConfigBase::Get()->Write("VCS", (long)VCS_AUTO);
  }
  
  return true;
}

#if wxUSE_GUI
wxStandardID wxExVCS::Request(wxWindow* parent)
{
  wxStandardID retValue;

  if ((retValue = ExecuteDialog(parent)) == wxID_OK)
  {
    ShowOutput();
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

  if (m_Command.IsCommit())
  {
    v.push_back(wxExConfigItem(
      _("Revision comment"), 
      CONFIG_COMBOBOX,
      wxEmptyString,
      true)); // required
  }

  if (m_Files.empty() && !m_Command.IsHelp())
  {
    v.push_back(wxExConfigItem(
      _("Base folder"), 
      CONFIG_COMBOBOXDIR, 
      wxEmptyString, 
      true,
      1000));

    if (m_Command.IsAdd())
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
void wxExVCS::ShowOutput(const wxString& caption) const
{
  const wxExFileName filename((!m_Files.empty() ? 
    m_Files[0]: wxString(wxEmptyString)));
  
  // Add a lexer when appropriate.
  if (m_Command.IsOpen() && !GetError() && !m_Command.IsHistory())
  {
    if (filename.GetLexer().IsOk())
    {
      GetDialog()->SetLexer(filename.GetLexer().GetScintillaLexer());
    }
  }
  else if (m_Command.IsDiff())
  {
    GetDialog()->SetLexer("diff");
  }
  else
  {
    GetDialog()->SetLexer(wxEmptyString);
  }

  wxString my_caption = m_Caption;
      
  if (!m_Command.IsHelp())
  {
    my_caption += " " + (filename.IsOk() ?  
      filename.GetFullName(): 
      wxExConfigFirstOf(_("Base folder")));
  }

  wxExCommand::ShowOutput(my_caption);
}
#endif

bool wxExVCS::SupportKeywordExpansion() const
{
  return FindVCSEntry((!m_Files.empty() ? 
    m_Files[0]: wxString(wxEmptyString))).SupportKeywordExpansion();
}

bool wxExVCS::Use() const
{
  return wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO) != VCS_NONE;
}
