////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.cpp
// Purpose:   Implementation of wex::vcs class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/vcs.h>
#include <wex/config.h>
#include <wex/itemdlg.h>
#include <wex/menus.h>
#include <wex/path.h>
#include <wex/util.h>

namespace wex
{
  // The vcs id's here can be set using the config dialog, and are not
  // present in the vcs.xml. 
  enum
  {
    VCS_NONE = -2, // no version control
    VCS_AUTO = -1, // uses the VCS appropriate for current file
    VCS_START = 0  // no where fixed VCS start (index in vector)
  };
};

std::vector<wex::vcs_entry> wex::vcs::m_Entries;

wex::vcs::vcs(const std::vector< path > & files, int command_no)
  : m_Files(files)
  , m_Title("VCS")
{
  m_Entry = FindEntry(get_file());
  
  if (!m_Entry.name().empty())
  {
    if (m_Entry.set_command(command_no))
    {
      m_Title = m_Entry.name() + " " + 
        m_Entry.get_command().get_command();
  
      if (!m_Entry.get_command().is_help() && m_Files.size() == 1)
      {
        m_Title += " " + path(m_Files[0]).fullname();
      }
    }
  }
}

int wex::vcs::config_dialog(const window_data& par) const
{
  if (m_Entries.empty())
  {
    return wxID_CANCEL;
  }
  
  item::choices_t choices{{(long)VCS_NONE, _("None")}};
  
  // Using auto vcs is not useful if we only have one vcs.
  if (m_Entries.size() > 1)
  {
    choices.insert({(long)VCS_AUTO, "Auto"});
  }
  
  long i = VCS_START;

  for (const auto& it : m_Entries)
  {
    choices.insert({i++, it.name()});
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

  // use a radiobox 
  std::vector<item> v{{"VCS", choices, true, cols}};

  for (const auto& it : m_Entries)
  {
    v.push_back({it.name(), item::FILEPICKERCTRL});
  }

  if (const window_data data(window_data(par).
    title(_("Set VCS").ToStdString())); data.button() & wxAPPLY)
  {
    item_dialog* dlg = new item_dialog(v, data);
    return dlg->Show();
  }
  else
  {
    return item_dialog(v, data).ShowModal();
  }
}

bool wex::vcs::dir_exists(const path& filename)
{
  if (const vcs_entry entry(FindEntry(filename));
    entry.admin_dir_is_toplevel() && 
    IsAdminDirTopLevel(entry.admin_dir(), filename))
  {
    return true;
  }
  else 
  {
    return IsAdminDir(entry.admin_dir(), filename);
  }
}

bool wex::vcs::execute()
{
  if (get_file().empty())
  {
    return m_Entry.execute(
      m_Entry.get_command().is_add() ? config(_("data")).get_firstof(): std::string(), 
      lexer(), 
      process::EXEC_WAIT,
      config(_("Base folder")).get_firstof());
  }
  else
  {
    const path filename(get_file());
    std::string args;
    path wd;
    
    if (m_Files.size() > 1)
    {
      for (const auto& it : m_Files)
      {
        args += "\"" + it.string() + "\" ";
      }
    }
    else if (m_Entry.name() == "git")
    {
      wd = filename.get_path();
      
      if (!filename.fullname().empty())
      {
        args = "./" + filename.fullname();
      }
    }
    else
    {
      args = "\"" + filename.string() + "\"";
    }
    
    return m_Entry.execute(args, 
      filename.lexer(), process::EXEC_WAIT, wd.string());
  }
}

bool wex::vcs::execute(const std::string& command)
{
  return m_Entry.execute(command, get_file().get_path());
}
  
const wex::vcs_entry wex::vcs::FindEntry(const std::string& filename) 
{
  return FindEntry(path(filename));
}

const wex::vcs_entry wex::vcs::FindEntry(const path& filename)
{
  if (const int vcs = config("VCS").get(VCS_AUTO);
    vcs == VCS_AUTO)
  {
    if (!filename.empty())
    {
      for (const auto& it : m_Entries)
      {
        const bool toplevel = it.admin_dir_is_toplevel();
        const std::string admin_dir = it.admin_dir();

        if (toplevel && IsAdminDirTopLevel(admin_dir, filename))
        {
          return it;
        }
        else if (IsAdminDir(admin_dir, filename))
        {
          return it;
        }
      }
    }
  }
  else if (vcs >= VCS_START && vcs < (int)m_Entries.size())
  {
    return m_Entries[vcs];
  }
  
  return vcs_entry();
}

const std::string wex::vcs::get_branch() const
{
  return config("VCS").get(VCS_AUTO) == VCS_NONE ?
    std::string(): 
    m_Entry.get_branch();
}

const wex::path wex::vcs::get_file() const
{
  return m_Files.empty() ? config(_("Base folder")).get_firstof(): m_Files[0];
}

const std::string wex::vcs::name() const
{
  switch (config("VCS").get(VCS_AUTO))
  {
    case VCS_NONE: return std::string();
    case VCS_AUTO: return "Auto";
    default: return m_Entry.name();
  }
}

// The .git dir only exists in the root, so check all components.

const std::string wex::vcs::GetRelativeFile(
  const std::string& admin_dir, const path& fn) const
{
  // .git
  // /home/user/wex/src/src/vi.cpp
  // should return -> src/src/vi.cpp

  // Ignore all common parts in fn, so parts that are in top level dir.
  auto it = std::find(fn.data().begin(), fn.data().end(), 
    GetTopLevelDir(admin_dir, fn).fullname());

  if (it == fn.data().end())
  {
    return std::string();
  }

  // The rest from fn is used for relative file.
  path relative_file;

  while (it != fn.data().end())
  {
    relative_file.append(*it++);
  }

  return relative_file.string();
}

const wex::path wex::vcs::GetTopLevelDir(
  const std::string& admin_dir, const path& fn)
{
  // .git
  // /home/user/wex/src/src/vi.cpp
  // should return -> /home/user/wex
  path root;

  for (const auto & p : fn.data())
  {
    if (IsAdminDir(admin_dir, root.append(p)))
    {
      return root;
    }
  }

  return path();
}

bool wex::vcs::IsAdminDir(const std::string& admin_dir, const path& fn)
{
  return 
    !admin_dir.empty() && !fn.empty() &&
     path(fn.get_path()).append(admin_dir).dir_exists();
}

bool wex::vcs::IsAdminDirTopLevel(
  const std::string& admin_dir, const path& fn)
{
  return !GetTopLevelDir(admin_dir, fn).empty();
}

bool wex::vcs::load_document()
{
  const auto old_entries = m_Entries.size();
  
  if (!menus::load("vcs", m_Entries)) return false;

  log::verbose("vcs entries") << m_Entries.size();
  
  if (old_entries == 0)
  {
    // Add default VCS.
    if (config c("VCS"); !c.exists())
    {
      c.set(VCS_AUTO);
    }
  }
  else if (old_entries != m_Entries.size())
  {
    // If current number of entries differs from old one,
    // we added or removed an entry. That might give problems
    // with the vcs id stored in the config, so reset it. 
    config("VCS").set(VCS_AUTO);
  }
  
  return true;
}

wxStandardID wex::vcs::request(const window_data& data)
{
  if (show_dialog(data) == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }  
  
  if (!execute())
  {
    return wxID_CANCEL;
  }
    
  m_Entry.show_output(m_Title);

  return wxID_OK;
}

bool wex::vcs::set_entry_from_base(wxWindow* parent)
{
  if (!use())
  {
    return false;
  }
  
  const std::string message = _("Select VCS Folder").ToStdString();
      
  if (
    // See also vcs_entry, same item is used there.
    const std::vector<item> v{{
      _("Base folder"), item::COMBOBOX_DIR, std::any(), control_data().is_required(true)}};
    config(_("Base folder")).get_firstof().empty()) 
  {
    if (
      parent != nullptr && 
      item_dialog(v, window_data().
        parent(parent).
        title(message)).ShowModal() == wxID_CANCEL)
    {
      return false;
    }
    
    m_Entry = FindEntry(config(_("Base folder")).get_firstof());
  }
  else
  {
    m_Entry = FindEntry(config(_("Base folder")).get_firstof());
  
    if (m_Entry.name().empty())
    {
      if (
        parent != nullptr &&
        item_dialog(v, window_data().
          parent(parent).
          title(message)).ShowModal() == wxID_CANCEL)
      {
        return false;
      }
      
      m_Entry = FindEntry(config(_("Base folder")).get_firstof());
    }
  }
  
  return !m_Entry.name().empty();
}

int wex::vcs::show_dialog(const window_data& arg)
{
  if (m_Entry.get_command().get_command().empty())
  {
    return wxID_CANCEL;
  }
  
  window_data data(window_data(arg).title(m_Title));
  const bool add_folder(m_Files.empty());

  if (m_Entry.get_command().ask_flags())
  {
    config(_("Flags")).set(config(m_Entry.flags_key()).get());
  }

  if (m_ItemDialog != nullptr)
  {
    data.pos(m_ItemDialog->GetPosition());
    data.size(m_ItemDialog->GetSize());
    delete m_ItemDialog;
    m_ItemDialog = nullptr;
  }

  const std::vector <item> v({
    m_Entry.get_command().is_commit() ? 
      item(_("Revision comment"), item::COMBOBOX, std::any(), control_data().is_required(true)): 
      item(),
    add_folder && !m_Entry.get_command().is_help() ? 
      item(_("Base folder"), item::COMBOBOX_DIR, std::any(), control_data().is_required(true)): 
      item(),
    add_folder && !m_Entry.get_command().is_help() && m_Entry.get_command().is_add() ? item(
      _("Path"), item::COMBOBOX, std::any(), control_data().is_required(true)): 
      item(), 
    m_Entry.get_command().ask_flags() ?  
      item(_("Flags"), std::string(), item::TEXTCTRL, control_data(), item::LABEL_LEFT, 
        [=](wxWindow* user, const std::any& value, bool save) {
          config(m_Entry.flags_key()).set(m_Entry.get_flags());}): 
      item(),
    m_Entry.flags_location() == vcs_entry::FLAGS_LOCATION_PREFIX &&
    m_Entry.get_command().ask_flags() ?  
      item(_("Prefix flags"), std::string()): 
      item(),
    m_Entry.get_command().use_subcommand() ? 
      item(_("Subcommand"), std::string()): 
      item()});

  bool all_empty = true;

  for (const auto& i : v)
  {
    if (i.type() != item::EMPTY)
    {
      all_empty = false;
    }
  }

  if (all_empty)
  {
    return wxID_OK;
  }

  m_ItemDialog = new item_dialog(v, data);

  return (data.button() & wxAPPLY) ? m_ItemDialog->Show(): m_ItemDialog->ShowModal();
}
  
bool wex::vcs::use() const
{
  return config("VCS").get(VCS_AUTO) != VCS_NONE;
}
