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

// The vcs id's here can be set using the config dialog, and are not
// present in the vcs.xml. 
enum
{
  VCS_NONE = 0, // no version control
  VCS_AUTO,     // uses the VCS appropriate for current file
  VCS_MAX
};

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
  const wxExFileName filename(GetFile());

  if (!filename.IsOk())
  {
    wxString args;

    if (m_Entry.GetCommand().IsAdd())
    {
      args = wxExConfigFirstOf(_("Path"));
    }
    
    return m_Entry.Execute(args, wxExLexer(), wxExConfigFirstOf(_("Base folder")));
  }
  else
  {
    wxString args;
    wxString wd;
    
    if (m_Files.size() > 1)
    {
      // File should not be surrounded by double quotes.
      for (
        auto it = m_Files.begin();
        it != m_Files.end();
        it++)
      {
        args += *it + " ";
      }
    }
    else if (m_Entry.GetName() == "git")
    {
      const wxString vcs = FindEntry(filename).GetName();
      wd = GetRoot(vcs, filename);
      
      if (!filename.GetFullName().empty())
      {
        args = GetRelativeFile(
          vcs, 
          filename);
      }
    }
    else if (m_Entry.GetName() == "SCCS")
    {
      args = "\"" + 
      // SCCS for windows does not handle windows paths,
      // so convert them to UNIX, and add volume as well.
#ifdef __WXMSW__      
        filename.GetVolume() + filename.GetVolumeSeparator() +
#endif        
        filename.GetFullPath(wxPATH_UNIX) + "\"";
    }
    else
    {
      args = "\"" + filename.GetFullPath() + "\"";
    }
    
    return m_Entry.Execute(args, filename.GetLexer(), wd);
  }
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
  
  const wxString message = _("Select VCS Folder");
  
  std::vector<wxExConfigItem> v;

  // See also vcsentry, same item is used there.
  v.push_back(wxExConfigItem(
    _("Base folder"), 
    CONFIG_COMBOBOXDIR, 
    wxEmptyString, 
    true,
    1000));
      
  if (wxExConfigFirstOf(_("Base folder")).empty()) 
  {
    if (wxExConfigDialog(parent, v, message).ShowModal() == wxID_CANCEL)
    {
      return false;
    }
  }
  else
  {
    m_Entry = FindEntry(wxExConfigFirstOf(_("Base folder")));
  
    if (m_Entry.GetName().empty())
    {
      if (wxExConfigDialog(parent, v, message).ShowModal() == wxID_CANCEL)
      {
        return false;
      }
    }
  }
  
  m_Entry = FindEntry(wxExConfigFirstOf(_("Base folder")));
  
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

// See CheckPathAll
const wxString wxExVCS::GetRelativeFile(
  const wxString& vcs, 
  const wxFileName& fn) const
{
  const wxString use_vcs = (vcs == "mercurial" ? "hg": vcs);

  // The .git dir only exists in the root, so check all components.
  wxFileName root(fn);
  wxArrayString as;

  while (root.DirExists() && root.GetDirCount() > 0)
  {
    wxFileName path(root);
    path.AppendDir("." + use_vcs);

    if (path.DirExists() && !path.FileExists())
    {
      wxString relative_file;

      for (int i = as.GetCount() - 1; i >= 0; i--)
      {
        relative_file += as[i] + "/";
      }
      
      return relative_file + fn.GetFullName();
    }

    as.Add(root.GetDirs().Last());
    root.RemoveLastDir();
  }
  
  return wxEmptyString;
}

// See CheckPathAll
const wxString wxExVCS::GetRoot(
  const wxString& vcs, 
  const wxFileName& fn) const
{
  if (!fn.IsOk() || vcs.empty())
  {
    return wxEmptyString;
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
      return root.GetPath();
    }

    root.RemoveLastDir();
  }

  return wxEmptyString;
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
