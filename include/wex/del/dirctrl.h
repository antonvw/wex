////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.h
// Purpose:   Declaration of class wex::del::dirctrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/window.h>
#include <wex/path.h>
#include <wx/generic/dirctrlg.h>

namespace wex::del
{
  class frame;

  /// Offers our generic dir control.
  /// It adds a popup menu and handling of the commands.
  class dirctrl : public wxGenericDirCtrl
  {
  public:
    /// Default constructor.
    dirctrl(
      del::frame*         frame,
      const data::window& data =
        data::window().style(wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_MULTIPLE));

    /// Expands path and selects it.
    void expand_and_select_path(const path& path);
  };
}; // namespace wex::del
