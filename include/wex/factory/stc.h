////////////////////////////////////////////////////////////////////////////////
// Name:      factory/stc.h
// Purpose:   Declaration of class wex::factory::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/factory/ex-command.h>
#include <wex/factory/lexer.h>
#include <wex/factory/text-window.h>
#include <wx/print.h>
#include <wx/stc/stc.h>

#include <bitset>

namespace wex
{
class indicator;

namespace data
{
class control;
class stc;
}; // namespace data

namespace factory
{
/// Offers a styled text ctrl with:
/// - lexer support (syntax colouring, folding)
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

  typedef std::bitset<4> margin_t;

  /// Virtual interface.

  /// Adds text.
  virtual void add_text(const std::string& text) { AddText(text); }

  /// Appends text (to end).
  virtual void append_text(const std::string& text) { AppendText(text); }

  /// After pressing enter, starts new line at same place
  /// as previous line.
  virtual bool auto_indentation(int c) { return false; }

  // Clears the component: all text is cleared and all styles are reset.
  // Invoked by Open and do_file_new.
  // (Clear is used by scintilla to clear the selection).
  virtual void clear(bool set_savepoint = true) { ; }

  /// Enables or disables folding depending on fold property
  /// (default not implemented).
  virtual void fold(
    /// if document contains more than 'Auto fold' lines,
    /// or if fold_all (and fold property is on) is specified,
    /// always all lines are folded.
    bool fold_all = false)
  {
    ;
  };

  /// Returns a ex command.
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
  virtual std::string get_hexmode_lines(const std::string& text)
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

  /// Returns line on which text margin was clicked,
  /// or -1 if not.
  virtual int get_margin_text_click() const { return -1; }

  /// Returns word at position.
  virtual const std::string get_word_at_pos(int pos) const
  {
    return std::string();
  };

  /// Injects data.
  virtual bool inject(const data::control& data) { return false; }

  // Inserts text at pos.
  virtual void insert_text(int pos, const std::string& text)
  {
    InsertText(pos, text);
  };

  /// Returns true if we are in hex mode (default false).
  virtual bool is_hexmode() const { return false; }

  /// Returns true if we are in visual mode (default true).
  virtual bool is_visual() const { return true; }

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

  /// Restores saved position.
  /// Returns true if position was saved before.
  virtual bool position_restore() { return false; }

  /// Saves position.
  virtual void position_save() { ; }

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

  /// Reset all margins.
  /// Default not implemented.
  virtual void reset_margins(margin_t type = margin_t().set()) { ; }

  /// Sets hex mode (default false).
  virtual bool set_hexmode(bool on) { return false; }

  /// Sets an indicator at specified start and end pos.
  /// Default false, not implemented.
  virtual bool set_indicator(const indicator& indicator, int start, int end)
  {
    return false;
  };

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

  /// Shows or hides line numbers.
  virtual void show_line_numbers(bool show) { ; }

  /// Starts or stops syncing.
  /// Default syncing is started during construction.
  virtual void sync(bool start = true) { ; }

  /// Use and show modification markers in the margin.
  /// If you open a file, the modification markers are used.
  virtual void use_modification_markers(bool use) { ; }

  /// Runs a vi command on this stc (default false).
  virtual bool vi_command(const std::string& command) { return false; }

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

  /// Returns EOL string.
  /// If you only want to insert a newline, use NewLine()
  /// (from wxStyledTextCtrl).
  const std::string eol() const;

  /// Returns current line fold level.
  size_t get_fold_level() const;

  /// Returns the lexer.
  const auto& get_lexer() const { return m_lexer; }

  /// Returns the lexer.
  auto& get_lexer() { return m_lexer; }

  /// Returns selected text as a string.
  const std::string get_selected_text() const;

  /// Returns the text.
  const std::string get_text() const
  {
    const wxCharBuffer& b(const_cast<factory::stc*>(this)->GetTextRaw());
    return std::string(b.data(), b.length());
  }

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
  void WordRightEndRectExtend() { ; }

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

private:
  lexer m_lexer;

  ex_command m_command;
};
}; // namespace factory
}; // namespace wex
