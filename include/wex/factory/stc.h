////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of class wex::factory::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/factory/ex-command.h>
#include <wex/factory/text-window.h>
#include <wex/factory/window.h>
#include <wx/print.h>
#include <wx/stc/stc.h>

#include <bitset>

namespace wex
{
class line_data;

namespace data
{
class control;
class stc;
}; // namespace data

namespace factory
{
/// Offers a basic styled text ctrl
class stc
  : public wxStyledTextCtrl
  , public text_window
{
public:
  /// Margin flags.
  enum
  {
    MARGIN_DIVIDER    = 0, ///< divider margin
    MARGIN_FOLDING    = 1, ///< folding margin
    MARGIN_LINENUMBER = 2, ///< line number margin
    MARGIN_TEXT       = 3, ///< text margin
  };

  /// A typedef containing margin flags.
  typedef std::bitset<4> margin_t;

  /// Default constructor.
  stc(const data::window& data = data::window());

  /// Virtual interface.

  /// Adds text.
  virtual void add_text(const std::string& text) { AddText(text); }

  /// Adds text block mode.
  virtual void add_text_block(const std::string& text) { AddText(text); }

  /// Appends text (to end).
  virtual void append_text(const std::string& text) { AppendText(text); }

  /// After pressing enter, starts new line at same place
  /// as previous line.
  virtual bool auto_indentation(int c) { return false; }

  /// Returns stc data.
  virtual wex::data::stc* get_data() { return nullptr; }

  /// Returns the ex command.
  virtual const ex_command& get_ex_command() const { return m_command; }

  /// Returns find string, from selected text or from config.
  /// The search flags are taken from frd.
  /// If text is selected, it also sets the find string.
  virtual const std::string get_find_string() const { return std::string(); }

  /// Hex erase.
  virtual bool get_hexmode_erase(int begin, int end) { return false; }

  /// Hex insert.
  virtual bool get_hexmode_insert(const std::string& command, int pos)
  {
    return false;
  };

  /// Hex lines.
  virtual std::string get_hexmode_lines(const std::string& text) const
  {
    return std::string();
  };

  /// Hex replace.
  virtual bool get_hexmode_replace(char) { return false; }

  /// Hex replace target.
  virtual bool
  get_hexmode_replace_target(const std::string& replacement, bool set_text)
  {
    return false;
  };

  /// Hex sync.
  virtual bool get_hexmode_sync() { return false; }

  /// Injects data.
  virtual bool inject(const data::control& data) { return false; }

  // Inserts text at pos.
  virtual void insert_text(int pos, const std::string& text)
  {
    InsertText(pos, text);
  };

  /// Returns true if we are in hex mode (default false).
  virtual bool is_hexmode() const { return false; }

  /// Returns true if we are in visual mode (not ex mode) (default true).
  virtual bool is_visual() const { return true; }

  /// Returns the name of the lexer.
  virtual std::string lexer_name() const { return std::string(); }

  /// Returns whether lexer is previewable.
  virtual bool lexer_is_previewable() const { return false; }

  /// If selected text is a link, opens the link.
  virtual bool link_open() { return false; }

  /// Opens the file, reads the content into the window,
  /// then closes the file and sets the lexer.
  virtual bool open(const wex::path& path, const data::stc& data)
  {
    return false;
  };

  /// Returns the path, as used by the file.
  /// Pure virtual, must be overridden.
  virtual const wex::path& path() const = 0;

  /// Prints the document.
  virtual void print(bool prompt = true) { ; }

  /// Shows a print preview.
  void virtual print_preview(
    wxPreviewFrameModalityKind kind = wxPreviewFrame_AppModal)
  {
    ;
  }

  /// Shows properties on the statusbar using specified flags.
  /// Default not implemented.
  virtual void
  properties_message(path::log_t flags = path::log_t().set(path::LOG_MOD))
  {
    ;
  }

  /// Sets hex mode (default false).
  virtual bool set_hexmode(bool on) { return false; }

  /// Sets search flags, default invokes SetSearchFlags.
  /// search flags to be used:
  /// - wxSTC_FIND_WHOLEWORD
  /// - wxSTC_FIND_MATCHCASE
  /// - wxSTC_FIND_WORDSTART
  /// - wxSTC_FIND_REGEXP
  /// - wxSTC_FIND_POSIX
  virtual void set_search_flags(int flags) { SetSearchFlags(flags); }

  /// Sets the text.
  virtual void set_text(const std::string& value) { SetText(value); }

  /// Shows decimal or hex info of word under cursor.
  virtual void show_ascii_value() { ; }

  /// Shows or hides line numbers.
  virtual void show_line_numbers(bool show) { ; }

  /// Starts or stops syncing.
  /// Default syncing is started during construction.
  virtual void sync(bool start = true) { ; }

  /// Use and show modification markers in the margin.
  /// If you open a file, the modification markers are used.
  virtual void use_modification_markers(bool use) { ; }

  /// Runs a vi command on this stc (default false).
  virtual bool vi_command(const line_data& data) { return false; }

  /// Finish last vi command (default false).
  virtual bool vi_command_finish(bool user_input) { return false; }

  /// Returns true if we are in vi visual mode (default false).
  virtual bool vi_is_visual() const { return false; }

  /// Returns vi mode as a string.
  virtual const std::string vi_mode() const { return std::string(); }

  /// Records a macro.
  virtual void vi_record(const std::string& command) { ; }

  /// Returns vi register.
  virtual std::string vi_register(char c) const { return std::string(); }

  /// Returns vi search flags.
  virtual int vi_search_flags() const { return 0; }

  /// Sets using visual vi (on) or ex mode (!on).
  virtual void visual(bool on) { ; }

  /// Other methods.

  /// Binds wx methods.
  void bind_wx();

  /// Clears the component: all text is cleared and all styles are reset.
  /// (Clear is used by scintilla to clear the selection).
  void clear(bool set_savepoint = true);

  /// Returns EOL string.
  /// If you only want to insert a newline, use NewLine()
  /// (from wxStyledTextCtrl).
  const std::string eol() const;

  /// Returns current line fold level.
  size_t get_fold_level() const;

  /// Returns line on which text margin was clicked,
  /// or -1 if not.
  int get_margin_text_click() const { return m_margin_text_click; }

  /// Returns selected text as a string.
  const std::string get_selected_text() const;

  /// Returns the text.
  const std::string get_text() const
  {
    const wxCharBuffer& b(const_cast<factory::stc*>(this)->GetTextRaw());
    return std::string(b.data(), b.length());
  }

  /// Returns word at position.
  const std::string get_word_at_pos(int pos) const;

  /// When clicked on a line with a text margin,
  /// returns revision id on the text margin, otherwise returns empty string.
  std::string margin_get_revision_id() const;

  /// Returns true if margin text is shown.
  bool margin_text_is_shown() const { return m_margin_text_is_shown; };

  /// Shows text margin.
  void margin_text_show() { m_margin_text_is_shown = true; };

  /// Restores saved position.
  /// Returns true if position was saved before.
  bool position_restore();

  /// Saves position.
  void position_save();

  /// Resets (all) margins.
  /// Default just resets all margins.
  void reset_margins(margin_t type = margin_t().set());

  /// Returns renamed.
  auto& vcs_renamed() const { return m_renamed; }

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
  void LineHome() { Home(); }
  void LineHomeExtend() { HomeExtend(); }
  void LineHomeRectExtend() { HomeRectExtend(); }
  void LineScrollDownExtend() { ; }
  void LineScrollDownRectExtend() { ; }
  void LineScrollUpExtend() { ; }
  void LineScrollUpRectExtend() { ; }
  void PageScrollDown();
  void PageScrollDownExtend() { ; }
  void PageScrollDownRectExtend() { ; }
  void PageScrollUp();
  void PageScrollUpExtend() { ; }
  void PageScrollUpRectExtend() { ; }
  void ParaUpRectExtend() { ; }
  void ParaDownRectExtend() { ; }
  void WordLeftRectExtend();
  void WordRightRectExtend();
  void WordRightEndRectExtend();

  /// Override methods from text_window.

  bool find(const std::string& text, int find_flags = -1, bool find_next = true)
    override;
  int get_current_line() const override
  {
    return const_cast<stc*>(this)->GetCurrentLine();
  };
  int  get_line_count() const override { return GetLineCount(); }
  int  get_line_count_request() override { return GetLineCount(); }
  void goto_line(int line) override;

protected:
  ex_command m_command;

  int  m_margin_text_click{-1};
  bool m_margin_text_is_shown{false};

  const int m_margin_divider_number{1}, m_margin_folding_number{2},
    m_margin_line_number{0}, m_margin_text_number{3};

  int m_saved_pos{-1}, m_saved_selection_start{-1}, m_saved_selection_end{-1};

  std::string m_renamed;
};
}; // namespace factory
}; // namespace wex
