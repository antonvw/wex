////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.cpp
// Purpose:   Implementation of wex::vcs class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/config.h>
#include <wex/item-dialog.h>
#include <wex/menus.h>
#include <wex/path.h>
#include <wex/util.h>
#include <wex/vcs.h>

namespace wex
{
  // The vcs id's here can be set using the config dialog, and are not
  // present in the vcs.xml.
  enum
  {
    VCS_NONE  = -2, // no version control
    VCS_AUTO  = -1, // uses the VCS appropriate for current file
    VCS_START = 0   // no where fixed VCS start (index in vector)
  };

  /// Offers several vcs admin support methods.
  class vcs_admin
  {
  public:
    /// Constructor.
    vcs_admin(const std::string& dir, const path& p)
      : m_dir(dir)
      , m_path(p)
    {
    }

    /// Returns true if admin dir exists for path.
    bool exists() const
    {
      return !m_dir.empty() && !m_path.empty() &&
             path(m_path).append(m_dir).dir_exists();
    }

    /// Returns true if toplevel is not empty.
    bool is_toplevel() const { return !toplevel().empty(); }

    /// Return toplevel dir.
    path toplevel() const
    {
      // .git
      // /home/user/wex/src/src/vi.cpp
      // should return -> /home/user/wex
      path root;

      for (const auto& part : m_path.data())
      {
        if (vcs_admin(m_dir, root.append(part)).exists())
        {
          return root;
        }
      }

      return path();
    }

  private:
    const std::string m_dir;
    const path        m_path;
  };

  const vcs_entry
  find_entry(const std::vector<vcs_entry>& entries, const path& p)
  {
    if (const int vcs = config("vcs.VCS").get(VCS_AUTO); vcs == VCS_AUTO)
    {
      if (!p.empty())
      {
        for (const auto& it : entries)
        {
          const vcs_admin va(it.admin_dir(), p);

          if (va.is_toplevel())
          {
            return it;
          }
          else if (va.exists())
          {
            return it;
          }
        }
      }
    }
    else if (vcs >= VCS_START && vcs < (int)entries.size())
    {
      return entries[vcs];
    }

    return vcs_entry();
  }

  const vcs_entry find_entry(const std::vector<vcs_entry>& entries)
  {
    return find_entry(
      entries,
      path(config(_("vcs.Base folder")).get_firstof()));
  }
}; // namespace wex

wex::vcs::vcs(const std::vector<path>& files, int command_no)
  : m_files(files)
  , m_title("VCS")
{
  m_entry = find_entry(m_entries, get_file());

  if (!m_entry.name().empty())
  {
    if (m_files.size() == 1 && !m_files[0].file_exists())
    {
      config(_("vcs.Base folder"))
        .set_firstof(
          vcs_admin(m_entry.admin_dir(), m_files[0]).toplevel().string());
    }

    if (m_entry.set_command(command_no))
    {
      m_title = m_entry.name() + " " + m_entry.get_command().get_command();

      if (!m_entry.get_command().is_help() && m_files.size() == 1)
      {
        m_title += " " + path(m_files[0]).fullname();
      }
    }
  }
}

int wex::vcs::config_dialog(const data::window& par) const
{
  if (m_entries.empty())
  {
    return wxID_CANCEL;
  }

  item::choices_t choices{{(long)VCS_NONE, _("None")}};

  // Using auto vcs is not useful if we only have one vcs.
  if (m_entries.size() > 1)
  {
    choices.insert({(long)VCS_AUTO, "Auto"});
  }

  long i = VCS_START;

  for (const auto& it : m_entries)
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
  std::vector<item> v{{"vcs.VCS", choices, true, data::item().columns(cols)}};

  for (const auto& it : m_entries)
  {
    v.push_back({"vcs." + it.name(), item::FILEPICKERCTRL});
  }

  if (const data::window data(
        data::window(par).title(_("Set VCS").ToStdString()));
      data.button() & wxAPPLY)
  {
    auto* dlg = new item_dialog(v, data);
    return dlg->Show();
  }
  else
  {
    return item_dialog(v, data).ShowModal();
  }
}

bool wex::vcs::dir_exists(const path& filename)
{
  if (const vcs_entry entry(find_entry(m_entries, filename));
      vcs_admin(entry.admin_dir(), filename).is_toplevel())
  {
    return true;
  }
  else
  {
    return vcs_admin(entry.admin_dir(), filename).exists();
  }
}

bool wex::vcs::execute()
{
  if (get_file().empty())
  {
    return m_entry.execute(
      m_entry.get_command().is_add() ? config(_("vcs.data")).get_firstof() :
                                       std::string(),
      lexer(),
      config(_("vcs.Base folder")).get_firstof());
  }
  else
  {
    const path  filename(get_file());
    std::string args;
    path        wd;

    if (m_files.size() > 1)
    {
      for (const auto& it : m_files)
      {
        args += "\"" + it.string() + "\" ";
      }
    }
    else if (m_entry.name() == "git")
    {
      wd = !filename.file_exists() ? filename : filename.get_path();

      if (filename.file_exists() && !filename.fullname().empty())
      {
        args = "\"" + filename.fullname() + "\"";
      }
    }
    else
    {
      args = "\"" + filename.string() + "\"";
    }

    return m_entry.execute(args, filename.lexer(), wd.string());
  }
}

bool wex::vcs::execute(const std::string& command)
{
  return m_entry.execute(command, get_file().get_path());
}

const std::string wex::vcs::get_branch() const
{
  return config("vcs.VCS").get(VCS_AUTO) == VCS_NONE ?
           std::string() :
           m_entry.get_branch(
             get_file().file_exists() ? get_file().get_path() :
                                        get_file().string());
}

const wex::path wex::vcs::get_file() const
{
  return m_files.empty() ? config(_("vcs.Base folder")).get_firstof() :
                           m_files[0];
}

bool wex::vcs::load_document()
{
  const auto old_entries = m_entries.size();

  if (!menus::load("vcs", m_entries))
    return false;

  log::verbose("vcs entries") << m_entries.size();

  if (old_entries == 0)
  {
    // Add default VCS.
    if (config c("vcs.VCS"); !c.exists())
    {
      c.set(VCS_AUTO);
    }
  }
  else if (old_entries != m_entries.size())
  {
    // If current number of entries differs from old one,
    // we added or removed an entry. That might give problems
    // with the vcs id stored in the config, so reset it.
    config("vcs.VCS").set(VCS_AUTO);
  }

  return true;
}

const std::string wex::vcs::name() const
{
  switch (config("vcs.VCS").get(VCS_AUTO))
  {
    case VCS_NONE:
      return std::string();
    case VCS_AUTO:
      return "Auto";
    default:
      return m_entry.name();
  }
}

wxStandardID wex::vcs::request(const data::window& data)
{
  if (show_dialog(data) == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  if (!execute())
  {
    return wxID_CANCEL;
  }

  m_entry.show_output(m_title);

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
    const std::vector<item> v{{_("vcs.Base folder"),
                               item::COMBOBOX_DIR,
                               std::any(),
                               data::control().is_required(true)}};

    config(_("vcs.Base folder")).get_firstof().empty())
  {
    if (
      parent != nullptr &&
      item_dialog(v, data::window().parent(parent).title(message)).ShowModal() ==
        wxID_CANCEL)
    {
      return false;
    }

    m_entry = find_entry(m_entries);
  }
  else
  {
    m_entry = find_entry(m_entries);

    if (m_entry.name().empty())
    {
      if (
        parent != nullptr &&
        item_dialog(v, data::window().parent(parent).title(message))
            .ShowModal() == wxID_CANCEL)
      {
        return false;
      }

      m_entry = find_entry(m_entries);
    }
  }

  return !m_entry.name().empty();
}

int wex::vcs::show_dialog(const data::window& arg)
{
  if (m_entry.get_command().get_command().empty())
  {
    return wxID_CANCEL;
  }

  data::window data(data::window(arg).title(m_title));
  const bool  add_folder(m_files.empty());

  if (m_entry.get_command().ask_flags())
  {
    config(_("vcs.Flags")).set(config(m_entry.flags_key()).get());
  }

  if (m_item_dialog != nullptr)
  {
    data.pos(m_item_dialog->GetPosition());
    data.size(m_item_dialog->GetSize());
    delete m_item_dialog;
    m_item_dialog = nullptr;
  }

  const std::vector<item> v(
    {m_entry.get_command().is_commit() ? item(
                                           _("vcs.Revision comment"),
                                           item::COMBOBOX,
                                           std::any(),
                                           data::control().is_required(true)) :
                                         item(),
     add_folder && !m_entry.get_command().is_help() ?
       item(
         _("vcs.Base folder"),
         item::COMBOBOX_DIR,
         std::any(),
         data::control().is_required(true)) :
       item(),
     add_folder && !m_entry.get_command().is_help() &&
         m_entry.get_command().is_add() ?
       item(
         _("vcs.Path"),
         item::COMBOBOX,
         std::any(),
         data::control().is_required(true)) :
       item(),
     m_entry.get_command().ask_flags() ?
       item(
         _("vcs.Flags"),
         std::string(),
         item::TEXTCTRL,
         data::item()
           .label_type(data::item::LABEL_LEFT)
           .apply([=](wxWindow* user, const std::any& value, bool save) {
             config(m_entry.flags_key()).set(m_entry.get_flags());
           })) :
       item(),
     m_entry.flags_location() == vcs_entry::FLAGS_LOCATION_PREFIX &&
         m_entry.get_command().ask_flags() ?
       item(_("vcs.Prefix flags"), std::string()) :
       item(),
     m_entry.get_command().use_subcommand() ?
       item(_("vcs.Subcommand"), std::string()) :
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

  m_item_dialog = new item_dialog(v, data);

  return (data.button() & wxAPPLY) ? m_item_dialog->Show() :
                                     m_item_dialog->ShowModal();
}

bool wex::vcs::use() const
{
  return config("vcs.VCS").get(VCS_AUTO) != VCS_NONE;
}
