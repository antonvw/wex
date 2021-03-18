////////////////////////////////////////////////////////////////////////////////
// Name:      factory/stc.h
// Purpose:   Declaration of class wex::factory::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <wex/ex-command.h>
#include <wex/factory/text-window.h>
#include <wex/path.h>
#include <wx/print.h>
#include <wx/stc/stc.h>

namespace wex
{
  class indicator;

  namespace data
  {
    class stc;
  };

  namespace factory
  {
    /// Offers a styled text ctrl.
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
      virtual const ex_command& get_ex_command() const { return m_command; };

      /// Returns current line number.
      virtual int get_current_line() const
      {
        return const_cast<stc*>(this)->GetCurrentLine();
      };

      /// Returns the filename, as used by the file.
      /// Pure virtual, must be overridden.
      virtual const path& get_filename() const = 0;

      /// Returns find string, from selected text or from config.
      /// The search flags are taken from frd.
      /// If text is selected, it also sets the find string.
      virtual const std::string get_find_string() { return std::string(); };

      /// Returns line on which text margin was clicked,
      /// or -1 if not.
      virtual int get_margin_text_click() const { return -1; };

      // Inserts text at pos.
      virtual void insert_text(int pos, const std::string& text) { ; };

      /// Returns true if we are in hex mode (default false).
      virtual bool is_hexmode() const { return false; };

      /// Returns true if we are in visual mode (default true).
      virtual bool is_visual() const { return true; };

      /// Opens the file, reads the content into the window,
      /// then closes the file and sets the lexer.
      virtual bool open(const path& filename, const data::stc& data)
      {
        return false;
      };

      /// Restores saved position.
      /// Returns true if position was saved before.
      virtual bool position_restore() { return false; };

      /// Saves position.
      virtual void position_save() { ; };

      /// Prints the document.
      virtual void print(bool prompt = true) { ; };

      /// Shows a print preview.
      void virtual print_preview(
        wxPreviewFrameModalityKind kind = wxPreviewFrame_AppModal)
      {
        ;
      }

      /// Shows properties on the statusbar using specified flags.
      /// Default not implemented.
      virtual void properties_message(path::status_t flags = 0) { ; };

      /// Reset all margins.
      /// Default not implemented.
      virtual void reset_margins(margin_t type = margin_t().set()) { ; };

      /// Sets hex mode (default false).
      virtual bool set_hexmode(bool on) { return false; };

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
      virtual void set_search_flags(int flags) { SetSearchFlags(flags); };

      /// Sets the text.
      virtual void set_text(const std::string& value) { SetText(value); };

      /// Starts or stops syncing.
      /// Default syncing is started during construction.
      virtual void sync(bool start = true) { ; };

      /// Runs a vi command on this stc (default false).
      virtual bool vi_command(const std::string& command) { return false; };

      /// Returns vi mode as a string.
      virtual const std::string vi_mode() const { return std::string(); };

      /// Records a macro.
      virtual void vi_record(const std::string& command) { ; };

      /// Returns vi register.
      virtual std::string vi_register(char c) const { return std::string(); };

      /// Returns vi search flags.
      virtual int vi_search_flags() const { return 0; };

      /// Other methods.

      /// Returns the lexer.
      const auto& get_lexer() const { return m_lexer; };

      /// Returns the lexer.
      auto& get_lexer() { return m_lexer; };

      /// Goes to specified line.
      void goto_line(int line) override
      {
        GotoLine(line);
        EnsureVisible(line);
        EnsureCaretVisible();
      };

      /// Other, normal methods.

      /// Returns EOL string.
      /// If you only want to insert a newline, use NewLine()
      /// (from wxStyledTextCtrl).
      const std::string eol() const;

      /// Returns selected text as a string.
      const std::string get_selected_text() const;

      /// Override methods from text_window.

      bool find(
        const std::string& text,
        int                find_flags = -1,
        bool               find_next  = true) override
      {
        return FindText(0, GetTextLength(), text, find_flags) !=
               wxSTC_INVALID_POSITION;
      };
      int get_line_count() const override { return GetLineCount(); };
      int get_line_count_request() override { return GetLineCount(); };

    private:
      // We use a separate lexer here as well
      // (though stc_file offers one), as you can manually override
      // the lexer.
      lexer m_lexer;

      ex_command m_command;
    };
  }; // namespace factory
};   // namespace wex

inline const std::string wex::factory::stc::eol() const
{
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR:
      return "\r";
    case wxSTC_EOL_CRLF:
      return "\r\n";
    case wxSTC_EOL_LF:
      return "\n";
    default:
      assert(0);
      break;
  }

  return "\r\n";
}

inline const std::string wex::factory::stc::get_selected_text() const
{
  const wxCharBuffer& b(const_cast<stc*>(this)->GetSelectedTextRaw());
  return b.length() == 0 ? std::string() :
                           std::string(b.data(), b.length() - 1);
}
