////////////////////////////////////////////////////////////////////////////////
// Name:      dialog.h
// Purpose:   Declaration of wex::dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wex/window-data.h>

namespace wex
{
  /// Offers a general dialog, with a separated button sizer at the bottom.
  /// Derived dialogs can use the user sizer for laying out their controls.
  class dialog : public wxDialog
  {
  public:
    /// Default constructor.
    dialog(const window_data& data = window_data());

    /// Returns the window data.
    const auto& data() const {return m_data;};
  protected:
    /// Adds to the user sizer using the sizer flags.
    wxSizerItem* add_user_sizer(
      wxWindow* window,
      const wxSizerFlags& flags = wxSizerFlags().Expand());

    /// Adds to the user sizer using the sizer flags.
    wxSizerItem* add_user_sizer(
      wxSizer* sizer,
      const wxSizerFlags& flags = wxSizerFlags().Expand());

    /// layouts the sizers. Should be invoked after adding to sizers.
    /// If you specified button flags,
    /// they will be put at the bottom of the top sizer,
    /// and a sepator line will be added as specified.
    void layout_sizers(bool add_separator_line = true);
  private:
    const window_data m_data;
    
    wxFlexGridSizer* m_top_sizer;
    wxFlexGridSizer* m_user_sizer;
  };
};
