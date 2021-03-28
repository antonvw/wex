////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Include file for wex::del::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <wex/del/defs.h>
#include <wex/del/listview.h>
#include <wex/file-history.h>
#include <wex/frame.h>

namespace wex
{
  class debug;
  class ex;
  class item_dialog;
}; // namespace wex

namespace wex::del
{
  class file;

  /// Adds file and project history support to frame.
  /// It also sets a change indicator in the title of the frame if applicable.
  /// Finally it adds find in files and selection dialogs.
  class frame : public wex::frame
  {
  public:
    /// Default constructor.
    /// Default it gives file history support to be used from the file menu.
    /// So you should call use_file_history_list somewhere to set it up.
    /// Default it does not use a recent project file.
    frame(
      size_t              maxFiles    = 9,
      size_t              maxProjects = 0,
      const data::window& data = data::window().style(wxDEFAULT_FRAME_STYLE));

    /// Virtual interface

    /// This method is called to activate a certain listview.
    /// Default it returns nullptr.
    virtual listview*
    activate(wex::data::listview::type_t, const lexer* lexer = nullptr)
    {
      return nullptr;
    };

    /// If there is a project somewhere,
    /// your implementation should return that one.
    /// Default it returns nullptr.
    virtual file* get_project() { return nullptr; };

    /// Other methods

    /// Returns a list with default file extensions.
    std::list<std::string> default_extensions() const;

    /// Finds (or replaces) in specified files.
    /// Returns true if process started.
    bool find_in_files(
      /// the files
      const std::vector<path>& files,
      /// ID_TOOL_REPORT_FIND or ID_TOOL_REPLACE
      int id,
      /// Default shows a dialog.
      bool show_dialog = true,
      /// report for output
      listview* report = nullptr);

    /// Shows a modal find (or replace) in files dialog.
    /// Returns result from ShowModal.
    int find_in_files_dialog(
      /// ID_TOOL_REPORT_FIND or ID_TOOL_REPLACE
      int id,
      /// add file types selection as well
      bool add_in_files = false);

    /// Returns caption for find_in_files_dialog.
    const std::string find_in_files_title(int id) const;

    /// Debugging interface.
    auto* get_debug() { return m_debug; };

    /// Returns project history.
    auto& get_project_history() { return m_project_history; };

    /// greps for text.
    /// The base directory is the directory for the current stc
    /// component, if available.
    /// Returns true if process started.
    bool grep(
      /// text [extension] [folder]
      const std::string& line,
      /// normally grep does not replace, by setting sed, it can
      bool sed = false);

    /// Sed (replace in files).
    /// The base directory is the directory for the current stc
    /// component, if available.
    /// Returns true if process started.
    bool sed(
      /// text replacement [extension] [folder]
      const std::string& line)
    {
      return grep(line, true);
    };

    /// Starts or stops syncing.
    /// Default syncing is started during construction.
    void sync(bool start);

    /// Updates project history.
    void set_recent_project(const path& path)
    {
      m_project_history.append(path);
    };

    /// Uses specified history list, and adds all elements from file history
    /// to the list.
    void use_file_history_list(listview* list);

    /// Overridden methods.

    void append_vcs(menu*, const menu_item* i) const override;
    void bind_accelerators(
      wxWindow*                              parent,
      const std::vector<wxAcceleratorEntry>& v,
      bool                                   debug = false) override;
    void          debug_add_menu(menu& m, bool b) override;
    void          debug_exe(int id, factory::stc* stc) override;
    void          debug_exe(const std::string& exe, factory::stc* stc) override;
    wxEvtHandler* debug_handler() override;
    bool          debug_is_active() const override;
    bool          debug_print(const std::string& text) override;
    bool          debug_toggle_breakpoint(int line, factory::stc* stc) override;

    void on_command_item_dialog(
      wxWindowID            dialogid,
      const wxCommandEvent& event) override;
    void          on_notebook(wxWindowID id, wxWindow* page) override;
    factory::stc* open_file(
      const path&      filename,
      const vcs_entry& vcs,
      const data::stc& data) override;
    void print_ex(wex::ex* ex, const std::string& text) override;
    void set_recent_file(const path& path) override;
    void show_ex_bar(int action = HIDE_BAR_FOCUS_STC, wex::ex* ex = nullptr)
      override;
    void show_ex_message(const std::string& text) override;
    void statusbar_clicked(const std::string&) override;
    void statusbar_clicked_right(const std::string&) override;

  protected:
    /// Access to file history list,
    /// if you use this as a page in a notebook,
    /// you might want prevent closing it.
    listview* file_history_list() { return m_file_history_listview; };

  private:
    void find_in_files(wxWindowID dialogid);
    void on_idle(wxIdleEvent& event);

    item_dialog *m_fif_dialog{nullptr}, *m_rif_dialog{nullptr};

    debug* m_debug{nullptr};

    listview*          m_file_history_listview{nullptr};
    class file_history m_project_history;

    static inline constexpr int id_find_in_files    = ID_FREE_LOWEST;
    static inline constexpr int id_replace_in_files = ID_FREE_LOWEST + 1;

    const std::string m_text_in_files{_("fif.In files")},
      m_text_in_folder{_("fif.In folder")},
      m_text_recursive{_("fif.Recursive")};

    // This set determines what fields are placed on the Find Files dialogs
    // as a list of checkboxes.
    const std::set<std::string> m_info;
  };
}; // namespace wex::del
