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
    virtual void after_sorting() override;
    virtual bool get_contents_changed() const override {return m_ContentsChanged;};
    virtual bool item_from_text(const std::string& text) override;
    virtual void reset_contents_changed() override {m_ContentsChanged = false;};

    /// Other methods
    
    /// Adds items. The items are added using a separate thread,
    /// default this thread runs detached, otherwise this method
    /// waits for the thread to finish.
    void add_items(
      const std::string& folder,
      const std::string& files,
      dir::type_t flags,
      bool detach = true);

    /// Returns the file.
    file& get_file() {return *this;};

    // Access to members.
    const auto& text_addfiles() const {return m_TextAddFiles;};
    const auto& text_addfolders() const {return m_TextAddFolders;};
    const auto& text_addrecursive() const {return m_TextAddRecursive;};
    const auto& text_addwhat() const {return m_TextAddWhat;};
    const auto& text_infolder() const {return m_TextInFolder;};
  private:
    virtual void build_popup_menu(menu& menu) override;
    virtual bool do_file_load(bool synced = false) override;
    virtual void do_file_new() override;
    virtual void do_file_save(bool save_as = false) override;
    void OnIdle(wxIdleEvent& event);

    bool m_ContentsChanged = false;
    
    const std::string 
      m_TextAddFiles = _("Add files"),
      m_TextAddFolders = _("Add folders"),
      m_TextAddRecursive = _("Recursive"),
      m_TextAddWhat = _("Add what"),
      m_TextInFolder = _("In folder");
    
    item_dialog* m_add_itemsDialog;
  };
};
