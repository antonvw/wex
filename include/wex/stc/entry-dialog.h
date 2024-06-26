////////////////////////////////////////////////////////////////////////////////
// Name:      stc-entry-dialog.h
// Purpose:   Declaration of class wex::stc_entry_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/stc.h>
#include <wex/ui/dialog.h>

#include <regex>
#include <string>

namespace wex
{
class stc;

/// Offers an stc as a dialog (like wxTextEntryDialog).
/// The prompt (if not empty) is first added as a text sizer to the user
/// sizer. Then the stc component is added to the user sizer.
class stc_entry_dialog : public dialog
{
public:
  /// Default constructor.
  stc_entry_dialog(
    /// initial text
    const std::string& text = std::string(),
    /// prompt (as with wxTextEntryDialog)
    const std::string& prompt = std::string(),
    /// dialog data with style:
    /// default wxDEFAULT_DIALOG_STYLE wxRESIZE_BORDER (set in dialog.cpp)
    const data::window& data = data::window(),
    /// stc data
    /// default multi line component
    const data::stc& stc_data = data::stc());

  /// Returns stc component.
  auto* get_stc() { return m_stc; }

  /// Sets a validator regular expression (case sensitive) for the stc contents.
  /// The validator is cleared after each valid wxID_OK, or after wxID_CANCEL.
  bool set_validator(
    /// the regex
    const std::string& regex,
    /// default the regex is case sensitive, set ic to override this
    bool ic = false);

private:
  std::regex  m_validator;
  std::string m_validator_string;

  wex::stc* m_stc;
};
}; // namespace wex
