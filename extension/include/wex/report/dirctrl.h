////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.h
// Purpose:   Declaration of class wex::report::dirctrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/generic/dirctrlg.h>
#include <wex/window-data.h>

namespace wex::report
{
  class frame;

  /// Offers our generic dir control.
  /// It adds a popup menu and handling of the commands.
  class dirctrl : public wxGenericDirCtrl
  {
  public:
    /// Default constructor.
    dirctrl(
      report::frame* frame,
      const wxString& filter = wxEmptyString, 
      int defaultFilter = 0,
      const window_data& data = window_data().
        style(wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_MULTIPLE));

    /// Expands path and selects it.
    void expand_and_select_path(const path& path);
  };
};
