////////////////////////////////////////////////////////////////////////////////
// Name:      listview-file.h
// Purpose:   Declaration of class wex::del::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/common/dir.h>
#include <wex/core/file.h>
#include <wex/del/listview.h>
#include <wex/ui/item-dialog.h>

namespace wex::del
{
/// Combines del::listview and file,
/// giving you a list control with file synchronization support.
class file
  : public listview
  , public wex::file
{
public:
  /// Constructor for a data::listview::FILE, opens the file.
  file(const wex::path& path, const data::listview& data = data::listview());

  /// Destructor.
  ~file() override;

  /// Other methods

  /// Adds items.
  void add_items(
    const std::string& folder,
    const std::string& files,
    data::dir::type_t  flags);

  /// Returns the file.
  file& get_file() { return *this; }

  // Access to members.
  const auto& text_addfiles() const { return m_text_add_files; }
  const auto& text_addfolders() const { return m_text_add_folders; }
  const auto& text_addrecursive() const { return m_text_add_recursive; }
  const auto& text_addwhat() const { return m_text_add_what; }
  const auto& text_infolder() const { return m_text_in_folder; }

  /// Virtual interface
  bool is_contents_changed() const final { return m_contents_changed; }
  bool item_from_text(const std::string& text) final;
  void reset_contents_changed() final { m_contents_changed = false; }

private:
  void after_sorting() final;
  void build_popup_menu(menu& menu) final;
  bool do_file_load(bool synced = false) final;
  void do_file_new() final;
  void do_file_save(bool save_as = false) final;

  void on_idle(wxIdleEvent& event);

  bool m_contents_changed = false;

  int m_old_count{0};

  const std::string m_text_add_files     = _("list.Add files"),
                    m_text_add_folders   = _("list.Add folders"),
                    m_text_add_recursive = _("list.Recursive"),
                    m_text_add_what      = _("list.Add what"),
                    m_text_in_folder     = _("list.In folder");

  item_dialog* m_add_items_dialog;
};
}; // namespace wex::del
