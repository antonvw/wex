////////////////////////////////////////////////////////////////////////////////
// Name:      dirctrl.h
// Purpose:   Declaration of class wex::del::dirctrl
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/factory/window.h>
#include <wx/generic/dirctrlg.h>

namespace wex::del
{
class frame;

/// Offers our generic dir control.
/// It adds a popup menu and handling of the commands.
class dirctrl : public wxGenericDirCtrl
{
public:
  /// Constructor.
  dirctrl(
    del::frame*         frame,
    const data::window& data =
      data::window().style(wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_MULTIPLE));

  /// Expands specified path and selects it.
  /// Returns false if path could not be expanded or selected.
  bool expand_and_select_path(const path& path);

  /// Returns vector with selections, empty if nothing is selected.
  std::vector<path>
    on_selected_paths(std::function<void(std::vector<path>)>) const;
};
}; // namespace wex::del
