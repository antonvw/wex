////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.cpp
// Purpose:   Implementation of wex::vcs class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/menus.h>
#include <wex/vcs/vcs.h>

#define SET_ENTRY                                                              \
  if (                                                                         \
    parent != nullptr && config(_("vcs.Always ask flags")).get(true) &&        \
    item_dialog(v, data::window().parent(parent).title(message))               \
        .ShowModal() == wxID_CANCEL)                                           \
  {                                                                            \
    return false;                                                              \
  }                                                                            \
                                                                               \
  m_entry = find_entry(m_store);

#include <map>
#include <numeric>

namespace wex
{
// The vcs id's here can be set using the config dialog, and are not
// present in the vcs.xml.
enum
{
  VCS_NONE  = -2, // no version control
  VCS_AUTO  = -1, // uses the VCS appropriate for current file
  VCS_START = 0   // number where fixed VCS start (index in vector)
};

vcs::store_t::iterator find_entry(vcs::store_t* store, const path& p)
{
  if (auto vcs = config("vcs.VCS").get(VCS_AUTO); vcs == VCS_AUTO)
  {
    if (!p.empty())
    {
      if (auto it = std::find_if(
            store->begin(),
            store->end(),
            [p](const auto& i)
            {
              const factory::vcs_admin va(i.admin_dir(), p);
              return va.is_toplevel() || va.exists();
            });
          it != store->end())
      {
        return it;
      }
    }
  }
  else if (vcs >= VCS_START && vcs < static_cast<int>(store->size()))
  {
    auto it = store->begin();
    std::advance(it, vcs);
    return it;
  }

  return store->begin(); // the empty vcs
}

vcs::store_t::iterator find_entry(vcs::store_t* store)
{
  return find_entry(store, path(config(_("vcs.Base folder")).get_first_of()));
}
}; // namespace wex

wex::vcs::vcs(const std::vector<wex::path>& files, int command_no)
  : m_files(files)
  , m_title("VCS")
{
  on_init();

  m_entry = find_entry(m_store, current_path());

  if (!m_entry->name().empty())
  {
    if (m_files.size() == 1 && !m_files[0].file_exists())
    {
      config(_("vcs.Base folder"))
        .set_first_of(factory::vcs_admin(m_entry->admin_dir(), m_files[0])
                        .toplevel()
                        .string());
    }

    if (m_entry->set_command(command_no))
    {
      m_title = m_entry->name() + " " + m_entry->get_command().get_command();

      if (!m_entry->get_command().is_help() && m_files.size() == 1)
      {
        m_title += " " + wex::path(m_files[0]).filename();
      }
    }
  }
}

int wex::vcs::config_dialog(const data::window& par) const
{
  if (m_store->empty())
  {
    return wxID_CANCEL;
  }

  item::choices_t choices{{(long)VCS_NONE, _("None")}};

  // Using auto vcs is not useful if we only have one vcs.
  if (m_store->size() > 1)
  {
    choices.insert({(long)VCS_AUTO, "Auto"});
  }

  for (long i = VCS_START; const auto& it : *m_store)
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

  config(_("vcs.Always ask flags")).get(true);
  config(_("vcs.Find includes submodules")).get(false);

  v.emplace_back(_("vcs.Always ask flags"), item::CHECKBOX);
  v.emplace_back(_("vcs.Find includes submodules"), item::CHECKBOX);

  std::transform(
    m_store->begin(),
    m_store->end(),
    std::back_inserter(v),
    [](const auto& t)
    {
      return item("vcs." + t.name(), item::FILEPICKERCTRL);
    });

  const data::window data(data::window(par).title(_("Set VCS").ToStdString()));
  if (data.button() & wxAPPLY)
  {
    auto* dlg = new item_dialog(v, data);
    return dlg->Show();
  }

  return item_dialog(v, data).ShowModal();
}

const wex::path wex::vcs::current_path() const
{
  if (m_files.empty())
  {
    return wex::path(config(_("vcs.Base folder")).get_first_of());
  }

  return m_files[0];
}

void wex::vcs::destroy_dialog()
{
  if (m_item_dialog != nullptr)
  {
    m_item_dialog->Destroy();
    m_item_dialog = nullptr;
  }
}

bool wex::vcs::dir_exists(const wex::path& filename)
{
  if (const auto& entry(find_entry(m_store, filename));
      factory::vcs_admin(entry->admin_dir(), filename).is_toplevel())
  {
    return true;
  }
  else
  {
    return factory::vcs_admin(entry->admin_dir(), filename).exists();
  }
}

bool wex::vcs::empty()
{
  return m_store->empty();
}

bool wex::vcs::execute()
{
  if (current_path().empty())
  {
    return m_entry->execute(
      m_entry->get_command().is_add() ? config(_("vcs.Path")).get_first_of() :
                                        std::string(),
      path(),
      config(_("vcs.Base folder")).get_first_of());
  }

  wex::path   wd(current_path());
  std::string args;

  if (m_files.size() > 1)
  {
    args = clipboard_add(std::accumulate(
      m_files.begin(),
      m_files.end(),
      std::string(),
      [](const std::string& a, const wex::path& b)
      {
        return a + quoted_find(b.string()) + " ";
      }));
  }
  else if (m_entry->name() == "git")
  {
    if (current_path().file_exists() && !current_path().filename().empty())
    {
      args = quoted_find(current_path().filename());
    }

    if (wd.file_exists())
    {
      wd = wex::path(wd.parent_path());
    }
  }
  else
  {
    args = quoted_find(current_path().string());
  }

  return m_entry->execute(args, current_path(), wd.string());
}

bool wex::vcs::execute(const std::string& command)
{
  return m_entry->system(
           process_data(command).start_dir(current_path().parent_path())) == 0;
}

const std::string wex::vcs::get_branch() const
{
  return config("vcs.VCS").get(VCS_AUTO) == VCS_NONE ?
           std::string() :
           m_entry->get_branch(
             current_path().file_exists() ? current_path().parent_path() :
                                            current_path().string());
}

bool wex::vcs::is_dir_excluded(const path& p) const
{
  return is_setup() && m_excludes.contains(p);
}

bool wex::vcs::is_file_excluded(const path& p) const
{
  // not yet implemented
  return false;
}

bool wex::vcs::load_document()
{
  on_init();

  const auto old_store = size();

  if (!menus::load("vcs", *m_store))
  {
    return false;
  }

  log::info("vcs entries") << size() << "from" << menus::path().string();

  if (old_store == 0)
  {
    // Add default VCS.
    if (config c("vcs.VCS"); !c.exists())
    {
      c.set(VCS_AUTO);
    }
  }
  else if (old_store != size())
  {
    // If current number of store differs from old one,
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
      return m_entry->name();
  }
}

void wex::vcs::on_exit()
{
  delete m_store;
  m_store = nullptr;

  // the m_item_dialog is owned by frame, no need to destroy it
  m_item_dialog = nullptr;
}

void wex::vcs::on_init()
{
  if (m_store == nullptr)
  {
    m_store = new store_t;
    m_store->emplace_back();

    load_document();
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

  m_entry->show_output(m_title);

  return wxID_OK;
}

void wex::vcs::set(const wex::path& p)
{
  m_files.clear();
  m_files.emplace_back(p);

  m_entry = find_entry(m_store, current_path());
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
    const std::vector<item> v{
      {_("vcs.Base folder"),
       item::COMBOBOX_DIR,
       std::any(),
       data::control().is_required(true)}};

    config(_("vcs.Base folder")).get_first_of().empty())
  {
    SET_ENTRY;
  }
  else
  {
    m_entry = find_entry(m_store);
    SET_ENTRY;
  }

  return !m_entry->name().empty();
}

bool wex::vcs::setup_exclude(const path& dir)
{
  if (config(_("vcs.Find includes submodules")).get(false))
  {
    return false;
  }

  const auto& s(m_entry->setup_exclude(toplevel(), dir));

  if (!s)
  {
    return false;
  }

  m_excludes = *s;
  set(dir);

  log::trace("vcs setup on") << dir.string() << "excludes" << m_excludes.size();

  return is_setup(true);
}

int wex::vcs::show_dialog(const data::window& arg)
{
  assert(!m_entry->name().empty());

  if (
    m_entry->get_command().get_command() == "grep" ||
    m_entry->get_command().get_command() == "show" ||
    (!config(_("vcs.Always ask flags")).get(true) &&
     m_entry->get_command().type().test(wex::menu_command::IS_ASKED)))
  {
    return wxID_OK;
  }

  if (m_entry->get_command().get_command().empty())
  {
    return wxID_CANCEL;
  }

  data::window data(data::window(arg).title(m_title));
  const bool   add_folder(m_files.empty());

  if (m_entry->get_command().ask_flags())
  {
    config(_("vcs.Flags")).set(config(m_entry->flags_key()).get());
  }

  if (m_item_dialog != nullptr)
  {
    data.pos(m_item_dialog->GetPosition());
    data.size(m_item_dialog->GetSize());
    delete m_item_dialog;
    m_item_dialog = nullptr;
  }

  const std::vector<item> v(
    {m_entry->get_command().is_commit() ? item(
                                            _("vcs.Revision comment"),
                                            item::COMBOBOX,
                                            std::any(),
                                            data::control().is_required(true)) :
                                          item(),
     add_folder && !m_entry->get_command().is_help() ?
       item(
         _("vcs.Base folder"),
         item::COMBOBOX_DIR,
         std::any(),
         data::control().is_required(true)) :
       item(),
     add_folder && !m_entry->get_command().is_help() &&
         m_entry->get_command().is_add() ?
       item(
         _("vcs.Path"),
         item::COMBOBOX,
         std::any(),
         data::control().is_required(true)) :
       item(),
     m_entry->get_command().ask_flags() ?
       item(
         _("vcs.Flags"),
         std::string(),
         item::TEXTCTRL,
         data::item()
           .label_type(data::item::LABEL_LEFT)
           .apply(
             [=, this](wxWindow* user, const std::any& value, bool save)
             {
               config(m_entry->flags_key()).set(m_entry->get_flags());
             })) :
       item(),
     m_entry->flags_location() == vcs_entry::flags_location_t::PREFIX &&
         m_entry->get_command().ask_flags() ?
       item(_("vcs.Prefix flags"), std::string()) :
       item(),
     m_entry->get_command().use_subcommand() ?
       item(_("vcs.Subcommand"), std::string()) :
       item()});

  if (std::all_of(
        v.begin(),
        v.end(),
        [](const auto& i)
        {
          return i.type() == item::EMPTY;
        }))
  {
    return wxID_OK;
  }

  m_item_dialog = new item_dialog(v, data);

  return (data.button() & wxAPPLY) ? m_item_dialog->Show() :
                                     m_item_dialog->ShowModal();
}

size_t wex::vcs::size()
{
  return m_store->size();
}

wex::path wex::vcs::toplevel() const
{
  return factory::vcs_admin(
           m_entry->admin_dir(),
           m_files.empty() ? current_path() : m_files[0])
    .toplevel();
}

bool wex::vcs::use() const
{
  return config("vcs.VCS").get(VCS_AUTO) != VCS_NONE;
}

bool wex::vcs_execute(
  factory::frame*          frame,
  int                      id,
  const std::vector<path>& files,
  const data::window&      data)
{
  if (files.empty())
  {
    return false;
  }

  if (vcs vcs(files, id); vcs.entry().get_command().is_open())
  {
    if (vcs.show_dialog(data) == wxID_OK)
    {
      std::for_each(
        files.begin(),
        files.end(),
        [frame, id](const auto& it)
        {
          if (wex::vcs vcs({it}, id); vcs.execute())
          {
            if (!vcs.entry().std_out().empty())
            {
              frame->open_file_vcs(it, vcs.entry(), data::stc());
            }
            else if (!vcs.entry().std_err().empty())
            {
              log() << vcs.entry().std_err();
            }
            else
            {
              log::status("No output");
              log::debug("no output from") << vcs.entry().data().exe();
            }
          }
        });
    }
  }
  else
  {
    vcs.request();
  }

  return true;
}
