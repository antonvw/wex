////////////////////////////////////////////////////////////////////////////////
// Name:      stc-entry-dialog.h
// Purpose:   Declaration of class wex::stc_entry_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/window.h>
#include <wex/dialog.h>

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
      /// data
      const data::window& data = data::window());

    /// Returns stc component.
    auto* get_stc() { return m_stc; };

  private:
    wex::stc* m_stc;
  };
}; // namespace wex
