////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.h
// Purpose:   Declaration of class wex::report::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/file.h>
#include <wex/itemdlg.h>
#include <wex/report/listview.h>

namespace wex::report
{
  /// Combines report::listview and file,
  /// giving you a list control with file synchronization support.
  class file : public listview, public wex::file
  {
  public:
    /// Constructor for a listview_data::FILE, opens the file.
    file(
      const std::string& file,
      const listview_data& data = listview_data());

    /// Destructor.
    virtual ~file();

    /// Virtual interface
    void after_sorting() override;
    bool get_contents_changed() const override {return m_ContentsChanged;};
    bool item_from_text(const std::string& text) override;
    void reset_contents_changed() override {m_ContentsChanged = false;};

    /// Other methods
    
    /// Adds items.
    void add_items(
      const std::string& folder,
      const std::string& files,
      dir::type_t flags);

    /// Returns the file.
    file& get_file() {return *this;};

    // Access to members.
    const auto& text_addfiles() const {return m_textAddFiles;};
    const auto& text_addfolders() const {return m_textAddFolders;};
    const auto& text_addrecursive() const {return m_textAddRecursive;};
    const auto& text_addwhat() const {return m_textAddWhat;};
    const auto& text_infolder() const {return m_textInFolder;};
  private:
    void build_popup_menu(menu& menu) override;
    bool do_file_load(bool synced = false) override;
    void do_file_new() override;
    void do_file_save(bool save_as = false) override;
    void on_idle(wxIdleEvent& event);

    bool m_ContentsChanged = false;
    
    const std::string 
      m_textAddFiles = _("Add files"),
      m_textAddFolders = _("Add folders"),
      m_textAddRecursive = _("Recursive"),
      m_textAddWhat = _("Add what"),
      m_textInFolder = _("In folder");
    
    item_dialog* m_add_itemsDialog;
  };
};
