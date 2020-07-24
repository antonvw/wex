////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <vector>
#include <wex/auto-complete.h>
#include <wex/hexmode.h>
#include <wex/item.h>
#include <wex/link.h>
#include <wex/marker.h>
#include <wex/stc-core.h>
#include <wex/stc-data.h>
#include <wex/stcfile.h>
#include <wex/vi.h>
#include <wx/prntbase.h>

namespace wex
{
  class indicator;
  class item;
  class item_dialog;
  class lexer;
  class managed_frame;
  class menu;
  class path;
  class vcs_entry;

  /// Offers a styled text ctrl with:
  /// - lexer support (syntax colouring, folding)
  /// - vi support (default vi mode is off)
  /// - find/replace
  /// - popup menu
  /// - printing
  class stc : public core::stc
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

    /// Virtual override methods.

    /// Will a cut succeed?
    bool CanCut() const override;

    /// Will a paste succeed?
    bool CanPaste() const override;

    /// Clear the selection.
    void Clear() override;

    /// Copies text to clipboard.
    void Copy() override;

    /// Cuts text to clipboard.
    void Cut() override;

    /// Paste text from clipboard.
    void Paste() override;

    /// Deselects selected text in the control.
    // Reimplemented, since scintilla version sets
    // empty sel at 0, and sets caret on pos 0.
    void SelectNone() override;

    /// If there is an undo facility and the last operation can be undone,
    /// undoes the last operation.
    void Undo() override;

    /// Virtual interface

    /// Processes specified char.
    /// Default does nothing, but is invoked during control_char_dialog,
    /// allowing you to add your own processing.
    /// Return true if char was processed.
    virtual bool process_char(int c) { return false; };

    /// Other methods.

    /// Adds text.
    void add_text(const std::string& text);

    /// Appends text (to end).
    void append_text(const std::string& text);

    /// Returns auto_complete.
    auto& auto_complete() { return m_auto_complete; };

    /// After pressing enter, starts new line at same place
    /// as previous line.
    bool auto_indentation(int c);

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

    /// Returns find string, from selected text or from config.
    /// The search flags are taken from frd.
    /// If text is selected, it also sets the find string.
    const std::string get_find_string();

    /// Returns current line fold level.
    int get_fold_level();

    /// Returns frame.
    auto get_frame() { return m_frame; };

    /// Returns hex mode component.
    const auto& get_hexmode() const { return m_hexmode; };

    /// Returns writable hex mode component.
    auto& get_hexmode() { return m_hexmode; };

    /// Returns the lexer.
    const auto& get_lexer() const { return m_lexer; };

    /// Returns the lexer.
    auto& get_lexer() { return m_lexer; };

    /// Returns line on which text margin was clicked,
    /// or -1 if not.
    auto get_margin_text_click() const { return m_margin_text_click; };

    /// Returns text.
    const std::string get_text() const;

    /// Returns vi component.
    const auto& get_vi() const { return m_vi; };

    /// Returns writable vi component.
    /// This allows you to do vi like editing:
    /// - get_vi().Command(":1,$s/xx/yy/g")
    /// - get_vi().Command(":w")
    /// to replace all xx by yy, and save the file.
    auto& get_vi() { return m_vi; };

    /// Returns word at position.
    const std::string get_word_at_pos(int pos) const;

    /// Returns true if line numbers are shown.
    bool is_shown_line_numbers() const
    {
      return GetMarginWidth(m_margin_line_number) > 0;
    };

    /// Keeps event data.
    void keep_event_data(bool synced) { m_data.event(synced); };

    /// If selected text is a link, opens the link.
    bool link_open();

    /// Deletes all change markers.
    /// Returns false if marker change is not loaded.
    bool marker_delete_all_change();

    /// Opens the file, reads the content into the window,
    /// then closes the file and sets the lexer.
    bool open(const path& filename, const data::stc& data = data::stc());

    /// Restores saved position.
    /// Returns true if position was saved before.
    bool position_restore();

    /// Saves position.
    void position_save();

    /// Prints the document.
    void print(bool prompt = true);

    /// Shows a print preview.
    void
    print_preview(wxPreviewFrameModalityKind kind = wxPreviewFrame_AppModal);

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

    /// Sets the text.
    void set_text(const std::string& value);

    /// Shows blame info for vcs in the text margin.
    /// Returns true if info was added.
    bool show_blame(const vcs_entry* vcs);

    /// Shows or hides line numbers.
    void show_line_numbers(bool show);

    /// Starts or stops syncing.
    /// Default syncing is started during construction.
    void sync(bool start = true);

    /// Use and show modification markers in the margin.
    /// If you open a file, the modification markers are used.
    void use_modification_markers(bool use);

    /// Virtual methods from core.

    const std::string eol() const override;
    bool              find_next(
                   const std::string& text,
                   int                find_flags = -1,
                   bool               find_next  = true) override;
    void        fold(bool fold_all = false) override;
    const path& get_filename() const override { return m_file.get_filename(); };
    const std::string get_selected_text() const override;
    bool is_hexmode() const override { return m_hexmode.is_active(); };
    void properties_message(path::status_t flags = 0) override;
    void reset_margins(margin_t type = margin_t().set()) override;
    bool set_hexmode(bool on) override { return get_hexmode().set(on); };
    bool set_indicator(const indicator& indicator, int start, int end) override;
    void set_search_flags(int flags) override;
    bool vi_command(const std::string& command) override
    {
      return m_vi.command(command);
    };

    // These methods are not yet available in scintilla, create stubs
    // (for the vi MOTION macro).
    void BigWordLeft();
    void BigWordLeftExtend();
    void BigWordLeftRectExtend();
    void BigWordRight();
    void BigWordRightEnd();
    void BigWordRightEndExtend();
    void BigWordRightEndRectExtend();
    void BigWordRightExtend();
    void BigWordRightRectExtend();
    void LineHome() { Home(); };
    void LineHomeExtend() { HomeExtend(); };
    void LineHomeRectExtend() { HomeRectExtend(); };
    void LineScrollDownExtend() { ; };
    void LineScrollDownRectExtend() { ; };
    void LineScrollUpExtend() { ; };
    void LineScrollUpRectExtend() { ; };
    void PageScrollDown();
    void PageScrollDownExtend() { ; };
    void PageScrollDownRectExtend() { ; };
    void PageScrollUp();
    void PageScrollUpExtend() { ; };
    void PageScrollUpRectExtend() { ; };
    void ParaUpRectExtend() { ; };
    void ParaDownRectExtend() { ; };
    void WordLeftRectExtend();
    void WordRightRectExtend();
    void WordRightEndRectExtend() { ; };

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

    managed_frame* m_frame;

    class auto_complete m_auto_complete;
    hexmode             m_hexmode;
    // We use a separate lexer here as well
    // (though stc_file offers one), as you can manually override
    // the lexer.
    lexer     m_lexer;
    data::stc m_data;
    stc_file  m_file;
    vi        m_vi;

    // All objects share the following:
    static inline item_dialog*       m_config_dialog = nullptr;
    static inline stc_entry_dialog*  m_entry_dialog  = nullptr;
    static inline link*              m_link          = nullptr;
    static inline int                m_zoom          = -1;
    static inline std::vector<item>* m_config_items  = nullptr;
  };
}; // namespace wex
