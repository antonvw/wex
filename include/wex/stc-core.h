////////////////////////////////////////////////////////////////////////////////
// Name:      stc-core.h
// Purpose:   Declaration of class wex::core::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <wex/text-window.h>
#include <wx/stc/stc.h>

namespace wex
{
  class indicator;
  class path;

  namespace core
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

      /// Returns the filename, as used by the file.
      /// Pure virtual, must be overridden.
      virtual const path& get_filename() const = 0;

      /// Returns true if we are in hex mode (default false).
      virtual bool is_hexmode() const { return false; };

      /// Returns true if we are in visual mode (default true).
      virtual bool is_visual() const { return true; };

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

      /// Runs a vi command on this stc (default false).
      virtual bool vi_command(const std::string& command) { return false; };

      /// Override methods from text_window.

      /// Finds next or previous.
      bool find(
        /// text to find
        const std::string& text,
        /// search flags to be used:
        /// - wxSTC_FIND_WHOLEWORD
        /// - wxSTC_FIND_MATCHCASE
        /// - wxSTC_FIND_WORDSTART
        /// - wxSTC_FIND_REGEXP
        /// - wxSTC_FIND_POSIX
        /// - if -1, use flags from find replace data
        int find_flags = -1,
        /// finds next or previous
        bool find_next = true) override
      {
        return FindText(0, GetTextLength(), text, find_flags) !=
               wxSTC_INVALID_POSITION;
      };

      /// Returns number of lines.
      int get_line_count() const override { return GetLineCount(); };

      /// Request for number of lines.
      int get_line_count_request() override { return GetLineCount(); };

      /// Goes to specified line.
      void goto_line(int line) override
      {
        GotoLine(line);
        EnsureVisible(line);
        EnsureCaretVisible();
      };

      /// Other, normal methods.

      /// Returns selected text as a string.
      const std::string get_selected_text() const;

      /// Returns EOL string.
      /// If you only want to insert a newline, use NewLine()
      /// (from wxStyledTextCtrl).
      const std::string eol() const;
    };
  }; // namespace core
};   // namespace wex

inline const std::string wex::core::stc::eol() const
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

inline const std::string wex::core::stc::get_selected_text() const
{
  const wxCharBuffer& b(const_cast<stc*>(this)->GetSelectedTextRaw());
  return b.length() == 0 ? std::string() :
                           std::string(b.data(), b.length() - 1);
}
