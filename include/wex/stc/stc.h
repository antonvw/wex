////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unordered_map>
#include <vector>

#include <wex/core/function-repeat.h>
#include <wex/data/stc.h>
#include <wex/factory/unified-diffs.h>
#include <wex/stc/file.h>
#include <wex/stc/hexmode.h>
#include <wex/syntax/marker.h>
#include <wex/syntax/stc.h>
#include <wex/ui/item.h>
#include <wex/vi/vi.h>
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
class stc_entry_dialog;

namespace factory
{
class frame;
class unified_diff;
}; // namespace factory

/// Offers a syntax stc with:
/// - ex or vi support (default vi mode is on)
/// - find/replace
/// - popup menu
/// - printing
class stc : public syntax::stc
{
public:
  enum
  {
    LINK_CHECK     = 0, ///< only check link
    LINK_OPEN      = 1, ///< open link as stc component
    LINK_OPEN_MIME = 2, ///< open link by mime
  };

  /// A typedef containing lnk flags.
  typedef std::bitset<3> link_t;

  // Static interface

  /// Shows a dialog with options, returns dialog return code.
  /// If used modeless, it uses the dialog id as specified,
  /// so you can use that id in frame::on_command_item_dialog.
  static int config_dialog(const data::window& data = data::window());

  /// Returns config items.
  static auto* config_items() { return m_config_items; }

  /// Returns the config dialog.
  static auto* get_config_dialog() { return m_config_dialog; }

  /// Saves static data in config.
  /// Invoked once during app::on_exit.
  static void on_exit();

  /// Reads static data from config (e.g. zooming).
  /// Invoked once during app::OnInit.
  static void on_init();

  /// Default constructor, sets text if not empty.
  stc(
    const std::string& text = std::string(),
    const data::stc&   data = data::stc());

  /// Constructor, opens the file if it exists.
  explicit stc(const wex::path& p, const data::stc& data = data::stc());

  /// Destructor.
  ~stc() override;

  // Virtual interface

  /// Processes specified char.
  /// Default does nothing, but is invoked during control_char_dialog,
  /// allowing you to add your own processing.
  /// Return true if char was processed.
  virtual bool process_char(int c) { return false; }

  // Other methods.

  /// Returns auto_complete.
  auto* auto_complete() { return m_auto_complete; }

  /// Sets the configurable parameters to values currently in config.
  void config_get();

  /// Returns associated data.
  const auto& data() const { return m_data; }

  /// Returns diffs.
  const unified_diffs& diffs() const { return m_diffs; };

  /// Returns writable diffs.
  unified_diffs& diffs() { return m_diffs; };

  /// Shows a menu with current line type checked,
  /// and allows you to change it.
  void filetype_menu();

  /// Finds next with settings from find replace data.
  bool find_next(bool stc_find_string = true);

  /// Returns the file.
  auto& get_file() { return m_file; }

  /// Returns frame.
  auto get_frame() { return m_frame; }

  /// Returns hex mode component.
  const auto& get_hexmode() const { return m_hexmode; }

  /// Returns writable hex mode component.
  auto& get_hexmode() { return m_hexmode; }

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
  void keep_event_data(bool synced) { m_data.event(synced); }

  /// Returns true if selected text (or a link on the current line
  /// can be opened, and fills the filename with the link.
  bool link_open(link_t mode, std::string* filename = nullptr);

  /// Deletes all change markers.
  /// Returns false if marker change is not loaded.
  bool marker_delete_all_change();

  /// Replaces all text.
  /// It there is a selection, it replaces in the selection, otherwise
  /// in the entire document.
  /// Returns the number of replacements.
  int replace_all(
    const std::string& find_text,
    const std::string& replace_text);

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

  /// Update markers according to diff.
  bool unified_diff_set_markers(const factory::unified_diff* uni);

  // Virtual methods from wxWidgets.

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

  // Virtual methods from factory.

  void add_text(const std::string& text) override;

  void add_text_block(const std::string& text) override;

  void append_text(const std::string& text) override;

  bool auto_indentation(int c) override;

  bool find(const std::string& text, int find_flags = -1, bool find_next = true)
    override;

  void generic_settings() override;

  wex::data::stc* get_data() override { return &m_data; }

  const ex_command& get_ex_command() const override
  {
    return m_vi->get_command();
  };

  const std::string get_find_string() const override;

  bool        get_hexmode_erase(int begin, int end) override;
  bool        get_hexmode_insert(const std::string& command, int pos) override;
  std::string get_hexmode_lines(const std::string& text) const override;
  bool        get_hexmode_replace(char) override;
  bool get_hexmode_replace_target(const std::string& replacement, bool set_text)
    override;
  bool get_hexmode_sync() override;

  int get_current_line() const override;
  int get_line_count() const override;
  int get_line_count_request() override;

  void goto_line(int line) override;
  bool inject(const data::control& data) override;
  void insert_text(int pos, const std::string& text) override;
  bool is_hexmode() const override { return m_hexmode.is_active(); }
  bool is_visual() const override;
  bool link_open() override;
  bool open(const wex::path& p, const data::stc& data = data::stc()) override;

  const wex::path& path() const override { return m_file.path(); }

  void print(bool prompt = true) override;
  void print_preview(
    wxPreviewFrameModalityKind kind = wxPreviewFrame_AppModal) override;
  void
  properties_message(path::log_t t = path::log_t().set(path::LOG_MOD)) override;
  bool set_hexmode(bool on) override { return get_hexmode().set(on); }
  void set_search_flags(int flags) override;
  void set_text(const std::string& value) override;
  void show_ascii_value() override;
  void show_line_numbers(bool show) override;
  void show_whitespace(bool show) override;
  void sync(bool start = true) override { m_function_repeat.activate(start); }

  void use_modification_markers(bool use) override;

  void vcs_clear_diffs() override;

  bool        vi_command(const line_data& data) override;
  bool        vi_command_finish(bool user_input) override;
  void        vi_record(const std::string& command) override;
  bool        vi_is_recording() const override;
  bool        vi_is_visual() const override;
  std::string vi_register(char c) const override;
  int         vi_search_flags() const override { return m_vi->search_flags(); }
  const std::string vi_mode() const override;
  void              visual(bool on) override;

private:
  void bind_all();
  void bind_other();
  void blame_revision(const std::string& offset = std::string());
  void build_popup_menu(menu& menu);
  void build_popup_menu_edit(menu& menu);
  void build_popup_menu_link(menu& menu);
  void check_brace();
  bool check_brace(int pos);
  bool current_line_contains_diff_marker();
  void eol_action(const wxCommandEvent& event);
  void file_action(const wxCommandEvent& event);
  bool file_readonly_attribute_changed();
  void guess_type_and_modeline();
  void jump_action();
  void key_action(wxKeyEvent& event);
  void margin_action(wxStyledTextEvent& event);
  bool mark_diff(size_t line, const marker& marker);
  void mark_modified(const wxStyledTextEvent& event);
  void mouse_action(wxMouseEvent& event);
  void on_styled_text(wxStyledTextEvent& event);
  void show_properties();
  void sort_action(const wxCommandEvent& event);

  const marker m_marker_change{marker(1)}, m_marker_diff_add{marker(3)},
    m_marker_diff_change{marker(4)}, m_marker_diff_del{marker(5)};

  bool m_skip{false};

  frame* m_frame;

  class auto_complete* m_auto_complete;

  hexmode         m_hexmode;
  function_repeat m_function_repeat;
  data::stc       m_data;
  stc_file        m_file;
  unified_diffs   m_diffs;

  std::unordered_map<int, int> m_marker_identifiers;

  int m_selection_mode_copy{wxSTC_SEL_STREAM};

  // The ex or vi component.
  vi* m_vi{nullptr};

  // All objects share the following:
  static inline item_dialog*       m_config_dialog = nullptr;
  static inline stc_entry_dialog*  m_prop_dialog   = nullptr;
  static inline link*              m_link          = nullptr;
  static inline int                m_zoom          = -1;
  static inline std::vector<item>* m_config_items  = nullptr;
};
}; // namespace wex
