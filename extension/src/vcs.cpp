////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.cpp
// Purpose:   Implementation of wxExVCS class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/xml/xml.h>
#include <wx/extension/vcs.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/util.h>

std::map<wxString, wxExVCSEntry> wxExVCS::m_Entries;
wxFileName wxExVCS::m_FileName;

wxExVCS::wxExVCS()
{
}

wxExVCS::wxExVCS(const wxArrayString& files, int menu_id)
  : m_Files(files)
{
  m_Entry = FindEntry(GetFile());
  
  if (!m_Entry.GetName().empty())
  {
    m_Entry.SetCommand(menu_id);
    m_Caption = m_Entry.GetName() + " " + m_Entry.GetCommand().GetCommand();
  
    if (!m_Entry.GetCommand().IsHelp() && m_Files.size() == 1)
    {
      m_Caption += " " + m_Files[0];
    }
  }
  else
  {
    m_Caption = "VCS";
  }
}

bool wxExVCS::CheckPath(const wxString& vcs, const wxFileName& fn)
{
  if (vcs.empty() || !fn.IsOk())
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
    return path.DirExists() && !path.FileExists();
  }
}

bool wxExVCS::CheckPathAll(
  const wxString& vcs, 
  const wxFileName& fn)
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

    if (path.DirExists() && !path.FileExists())
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

bool wxExVCS::DirExists(const wxFileName& filename)
{
  const wxString vcs = FindEntry(filename).GetName();

  if (IsCheckPathAllVCS(vcs) && CheckPathAll(vcs, filename))
  {
    return true;
  }
  else 
  {
    return CheckPath(vcs, filename);
  }
}

long wxExVCS::Execute()
{
  const wxFileName filename(GetFile());

  wxString wd;
  wxString file;

  if (!filename.IsOk())
  {
    wd = wxExConfigFirstOf(_("Base folder"));

    if (m_Entry.GetCommand().IsAdd())
    {
      file = wxExConfigFirstOf(_("Path"));
    }
  }
  else
  {
    if (m_Files.size() > 1)
    {
      // File should not be surrounded by double quotes.
      for (
        auto it = m_Files.begin();
        it != m_Files.end();
        it++)
      {
        file += *it + " ";
      }
    }
    else if (m_Entry.GetName() == "git")
    {
      wd = filename.GetPath();
      
      if (!filename.GetFullName().empty())
      {
        file = "\"" + filename.GetFullName() + "\"";
      }
    }
    else if (m_Entry.GetName() == "SCCS")
    {
      file = "\"" + 
      // SCCS for windows does not handle windows paths,
      // so convert them to UNIX, and add volume as well.
#ifdef __WXMSW__      
        filename.GetVolume() + filename.GetVolumeSeparator() +
#endif        
        filename.GetFullPath(wxPATH_UNIX) + "\"";
    }
    else
    {
      file = "\"" + filename.GetFullPath() + "\"";
    }
  }

  return m_Entry.Execute(wxExFileName(GetFile()).GetLexer(), file, wd);
}

const wxExVCSEntry wxExVCS::FindEntry(const wxFileName& filename)
{
  const int vcs = wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO);

  if (vcs == VCS_AUTO)
  {
    if (filename.IsOk())
    {
      for (
        auto it = m_Entries.begin();
        it != m_Entries.end();
        ++it)
      {
        const wxString name = it->second.GetName();

        if (IsCheckPathAllVCS(name) && CheckPathAll(name, filename))
        {
          return it->second;
        }
        else if (CheckPath(name, filename))
        {
          return it->second;
        }
      }
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
  
  return wxExVCSEntry();
}

bool wxExVCS::GetDir(wxWindow* parent)
{
  if (!Use())
  {
    return false;
  }
  
  wxString folder = wxExConfigFirstOf(_("Base folder"));
  const wxString message = _("Select VCS Folder");
  
  if (folder.empty()) 
  {
    wxDirDialog dlg(parent, message);
    
    if (dlg.ShowModal() == wxID_CANCEL)
    {
      return false;
    }
    
    folder = dlg.GetPath() + wxFileName::GetPathSeparator();
    
    wxConfigBase::Get()->Write(_("Base folder"), folder);
  }
  
  m_Entry = FindEntry(folder);
  
  if (m_Entry.GetName().empty())
  {
    wxDirDialog dlg(
      parent,
      message,
      folder);
    
    if (dlg.ShowModal() == wxID_CANCEL)
    {
      return false;
    }
    
    folder = dlg.GetPath() + wxFileName::GetPathSeparator();
    
    wxConfigBase::Get()->Write(_("Base folder"), folder);
  }
  
  m_Entry = FindEntry(folder);
  
  return !m_Entry.GetName().empty();
}

const wxString wxExVCS::GetFile() const
{
  if (m_Files.empty())
  {
    return wxExConfigFirstOf(_("Base folder"));
  }
  else
  {
    return m_Files[0];
  }
}

const wxString wxExVCS::GetName() const
{
  if (!Use())
  {
    return wxEmptyString;
  }
  else if (wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO) == VCS_AUTO)
  {
    return "Auto";
  }
  else
  {
    return m_Entry.GetName();
  }
}

bool wxExVCS::IsCheckPathAllVCS(const wxString& vcs)
{
  return vcs == "git" || vcs == "mercurial";
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

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
    if (child->GetName() == "vcs")
    {
      const wxExVCSEntry vcs(child, m_Entries.size() + VCS_MAX);
      m_Entries.insert(std::make_pair(vcs.GetName(), vcs));
    }

    child = child->GetNext();
  }

  if (old_entries == 0)
  {
    // Add default VCS.
    if (!wxConfigBase::Get()->Exists("VCS"))
    {
      wxConfigBase::Get()->Write("VCS", (long)VCS_AUTO);
    }
  }
  else if (old_entries != m_Entries.size())
  {
    // If current number of entries differs from old one,
    // we added or removed an entry. That might give problems
    // with the vcs id stored in the config, so reset it. 
    wxConfigBase::Get()->Write("VCS", (long)VCS_AUTO);
  }
  
  return true;
}

#if wxUSE_GUI
wxStandardID wxExVCS::Request(wxWindow* parent)
{
  if (ShowDialog(parent) == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }  
  
  if (Execute() < 0)
  {
    return wxID_CANCEL;
  }
    
  m_Entry.ShowOutput(m_Caption);

  return wxID_OK;
}
#endif

bool wxExVCS::Use() const
{
  return wxConfigBase::Get()->ReadLong("VCS", VCS_AUTO) != VCS_NONE;
}
