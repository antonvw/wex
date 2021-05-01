////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <vector>
#include <wex/data/dir.h>
#include <wex/data/stc.h>
#include <wex/factory/stc.h>
#include <wex/hexmode.h>
#include <wex/item.h>
#include <wex/marker.h>
#include <wex/stc-file.h>
#include <wex/vi.h>
#include <wx/prntbase.h>

namespace wex
{
  class auto_complete;
  class indicator;
  class item;
  class item_dialog;
  class link;
  class frame;
  class menu;
  class path;
  class stc_entry_dialog;
  class vcs_entry;

  namespace factory
  {
    class frame;
  };

  /// Offers a styled text ctrl with:
  /// - ex or vi support (default vi mode is on)
  /// - find/replace
  /// - popup menu
  /// - printing
  class stc : public factory::stc
  {
  public:
    /// Static interface

    /// Shows a dialog with options, returns dialog return code.
    /// If used modeless, it uses the dialog id as specified,
    /// so you can use that id in frame::on_command_item_dialog.
    static int config_dialog(const data::window& data = data::window());

    /// Returns config items.
    static auto* config_items() { return m_config_items; };

    /// Returns the config dialog.
    static auto* get_config_dialog() { return m_config_dialog; };

    /// Saves static data in config.
    /// Invoked once during app::on_exit.
    static void on_exit();

    /// Reads static data from config (e.g. zooming).
    /// Invoked once during app::OnInit.
    static void on_init();

    /// Constructors.

    /// Default constructor, sets text if not empty.
    stc(
      const std::string& text = std::string(),
      const data::stc&   data = data::stc());

    /// Constructor, opens the file if it exists.
    stc(const path& file, const data::stc& data = data::stc());

    /// Destructor.
    ~stc() override;

    /// Virtual interface

    /// Processes specified char.
    /// Default does nothing, but is invoked during control_char_dialog,
    /// allowing you to add your own processing.
    /// Return true if char was processed.
    virtual bool process_char(int c) { return false; };

    /// Other methods.

    /// Appends text (to end).
    void append_text(const std::string& text);

    /// Returns auto_complete.
    auto* auto_complete() { return m_auto_complete; };

    // Clears the component: all text is cleared and all styles are reset.
    // Invoked by Open and do_file_new.
    // (Clear is used by scintilla to clear the selection).
    void clear(bool set_savepoint = true);

    /// Sets the configurable parameters to values currently in config.
    void config_get();

    /// Returns associated data.
    const auto& data() const { return m_data; };

    /// Shows a menu with current line type checked,
    /// and allows you to change it.
    void filetype_menu();

    /// Finds next with settings from find replace data.
    bool find_next(bool stc_find_string = true);

    /// Returns the file.
    auto& get_file() { return m_file; };

    /// Returns frame.
    auto get_frame() { return m_frame; };

    /// Returns hex mode component.
    const auto& get_hexmode() const { return m_hexmode; };

    /// Returns writable hex mode component.
    auto& get_hexmode() { return m_hexmode; };

    /// Returns vi component.
    const vi& get_vi() const;

    /// Returns writable vi component.
    /// This allows you to do ex like editing:
    /// - get_vi().Command(":1,$s/xx/yy/g")
    /// - get_vi().Command(":w")
    /// to replace all xx by yy, and save the file.
    vi& get_vi();

    /// Returns true if line numbers are shown.
    bool is_shown_line_numbers() const
    {
      return GetMarginWidth(m_margin_line_number) > 0;
    };

    /// Keeps event data.
    void keep_event_data(bool synced) { m_data.event(synced); };

    /// Deletes all change markers.
    /// Returns false if marker change is not loaded.
    bool marker_delete_all_change();

    /// Replaces all text.
    /// It there is a selection, it replaces in the selection, otherwise
    /// in the entire document.
    /// Returns the number of replacements.
    int
    replace_all(const std::string& find_text, const std::string& replace_text);

    /// Replaces text and calls find next.
    /// Uses settings from find replace data.
    bool replace_next(bool stc_find_string = true);

    /// Replaces text and calls find next.
    /// It there is a selection, it replaces in the selection, otherwise
    /// it starts at current position.
    bool replace_next(
      /// text to find
      const std::string& find_text,
      /// text to replace with
      const std::string& replace_text,
      /// search flags to be used:
      /// - wxSTC_FIND_WHOLEWORD
      /// - wxSTC_FIND_MATCHCASE
      /// - wxSTC_FIND_WORDSTART
      /// - wxSTC_FIND_REGEXP
      /// - wxSTC_FIND_POSIX
      /// - if -1, use flags from find replace data
      int find_flags = 0,
      /// argument passed on to find_next
      bool stc_find_string = true);

    /// Shows blame info for vcs in the text margin.
    /// Returns true if info was added.
    bool show_blame(const vcs_entry* vcs);

    /// Virtual methods from wxWidgets.

    bool CanCut() const override;
    bool CanPaste() const override;
    void Clear() override;
    void Copy() override;
    void Cut() override;
    bool IsModified() const override;
    void Paste() override;
    // Reimplemented, since scintilla version sets
    // empty sel at 0, and sets caret on pos 0.
    void SelectNone() override;
    void Undo() override;

    /// Virtual methods from factory.

    void add_text(const std::string& text) override;

    void auto_complete_clear() override;
    void auto_complete_sync() override;
    bool auto_indentation(int c) override;

    bool find(
      const std::string& text,
      int                find_flags = -1,
      bool               find_next  = true) override;

    void fold(bool fold_all = false) override;

    const ex_command& get_ex_command() const override
    {
      return m_vi->get_command();
    };

    const path& get_filename() const override { return m_file.get_filename(); };

    const std::string get_find_string() const override;
    bool              get_hexmode_erase(int begin, int end) override;
    bool get_hexmode_insert(const std::string& command, int pos) override;
    bool get_hexmode_replace(char) override;
    bool get_hexmode_replace_target(
      const std::string& replacement,
      bool               set_text) override;
    bool get_hexmode_sync() override;

    int get_current_line() const override;
    int get_line_count() const override;
    int get_line_count_request() override;
    int get_margin_text_click() const override { return m_margin_text_click; };
    const std::string get_word_at_pos(int pos) const override;

    void goto_line(int line) override;
    void insert_text(int pos, const std::string& text) override;
    bool is_hexmode() const override { return m_hexmode.is_active(); };
    bool is_visual() const override;
    bool link_open() override;
    bool
    open(const path& filename, const data::stc& data = data::stc()) override;
    bool position_restore() override;
    void position_save() override;
    void print(bool prompt = true) override;
    void print_preview(
      wxPreviewFrameModalityKind kind = wxPreviewFrame_AppModal) override;
    void properties_message(path::status_t flags = 0) override;
    void reset_margins(margin_t type = margin_t().set()) override;
    bool set_hexmode(bool on) override { return get_hexmode().set(on); };
    bool set_indicator(const indicator& indicator, int start, int end) override;
    void set_search_flags(int flags) override;
    void set_text(const std::string& value) override;
    void show_line_numbers(bool show) override;
    void sync(bool start = true) override;
    void use_modification_markers(bool use) override;

    bool        vi_command(const std::string& command) override;
    void        vi_record(const std::string& command) override;
    std::string vi_register(char c) const override;
    int vi_search_flags() const override { return m_vi->search_flags(); };
    const std::string vi_mode() const override;
    void              visual(bool on) override;

  private:
    enum
    {
      LINK_CHECK     = 0,
      LINK_OPEN      = 1,
      LINK_OPEN_MIME = 2,
    };

    typedef std::bitset<3> link_t;

    void bind_all();
    void bind_other();
    void build_popup_menu(menu& menu);
    void check_brace();
    bool check_brace(int pos);
    void eol_action(const wxCommandEvent& event);
    void file_action(const wxCommandEvent& event);
    bool file_readonly_attribute_changed();
    void fold_all();
    void guess_type_and_modeline();
    void jump_action();
    void key_action(wxKeyEvent& event);
    bool link_open(link_t mode, std::string* filename = nullptr);
    void margin_action(wxStyledTextEvent& event);
    void mouse_action(wxMouseEvent& event);
    void mark_modified(const wxStyledTextEvent& event);
    void on_idle(wxIdleEvent& event);
    void on_styled_text(wxStyledTextEvent& event);
    void show_properties();
    void sort_action(const wxCommandEvent& event);

    const int m_margin_divider_number{1}, m_margin_folding_number{2},
      m_margin_line_number{0}, m_margin_text_number{3};

    const marker m_marker_change = marker(1);

    int m_fold_level{0}, m_margin_text_click{-1}, m_saved_pos{-1},
      m_saved_selection_start{-1}, m_saved_selection_end{-1};

    bool m_adding_chars{false}, m_skip{false};

    frame* m_frame;

    class auto_complete* m_auto_complete;
    hexmode              m_hexmode;

    data::stc m_data;
    stc_file  m_file;

    // The ex or vi component.
    vi* m_vi{nullptr};

    // All objects share the following:
    static inline item_dialog*       m_config_dialog = nullptr;
    static inline stc_entry_dialog*  m_prop_dialog   = nullptr;
    static inline link*              m_link          = nullptr;
    static inline int                m_zoom          = -1;
    static inline std::vector<item>* m_config_items  = nullptr;
  };

  /// Shows a dialog and opens selected files
  /// (calls open_files).
  void open_files_dialog(
    /// frame
    factory::frame* frame,
    /// flags to be used with file_dialog
    bool ask_for_continue = false,
    /// data to be used with open_file
    const data::stc& data = data::stc(),
    /// flags to be used with open_file_dir
    const data::dir::type_t& type = data::dir::type_t().set());
}; // namespace wex
