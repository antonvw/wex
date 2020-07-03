////////////////////////////////////////////////////////////////////////////////
// Name:      stc-core.h
// Purpose:   Declaration of class wex::core::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <wx/stc/stc.h>

namespace wex
{
  class indicator;
  class path;

  namespace core
  {
    /// Offers a styled text ctrl.
    class stc : public wxStyledTextCtrl
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

      /// Pure virtual interface.

      /// Returns EOL string.
      /// If you only want to insert a newline, use NewLine()
      /// (from wxStyledTextCtrl).
      virtual const std::string eol() const = 0;

      /// Finds next.
      virtual bool find_next(
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
        bool find_next = true) = 0;

      /// Enables or disables folding depending on fold property.
      virtual void fold(
        /// if document contains more than 'Auto fold' lines,
        /// or if fold_all (and fold property is on) is specified,
        /// always all lines are folded.
        bool fold_all = false) = 0;

      /// Returns the filename, as used by the file.
      virtual const path& get_filename() const = 0;

      /// Returns selected text as a string.
      virtual const std::string get_selected_text() const = 0;

      /// Returns true if we are in hex mode.
      virtual bool is_hexmode() const = 0;

      /// Shows properties on the statusbar using specified flags.
      virtual void properties_message(path::status_t flags = 0) = 0;

      /// Reset all margins.
      /// Default also resets the divider margin.
      virtual void reset_margins(margin_t type = margin_t().set()) = 0;

      /// Sets hex mode.
      virtual bool set_hexmode(bool on) = 0;

      /// Sets an indicator at specified start and end pos.
      virtual bool
      set_indicator(const indicator& indicator, int start, int end) = 0;

      /// search flags to be used:
      /// - wxSTC_FIND_WHOLEWORD
      /// - wxSTC_FIND_MATCHCASE
      /// - wxSTC_FIND_WORDSTART
      /// - wxSTC_FIND_REGEXP
      /// - wxSTC_FIND_POSIX
      /// - if -1, use flags from find replace data
      virtual void set_search_flags(int flags) = 0;

      /// Runs a vi command on this stc.
      virtual bool vi_command(const std::string& command) = 0;
    };
  }; // namespace core
};   // namespace wex
