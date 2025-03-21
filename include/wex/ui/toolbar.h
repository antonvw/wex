////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.h
// Purpose:   Declaration of wex::toolbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2010-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/toolbar-item.h>
#include <wex/factory/window.h>
#include <wx/aui/auibar.h>

#include <tuple>
#include <vector>

class wxCheckBox;

namespace wex
{
class find_bar;
class frame;

/// Offers a toolbar together with wex art.
/// Default no controls are added, you have to call
/// one of the add_ methods to do that.
class toolbar : public wxAuiToolBar
{
public:
  /// Type for adding checkboxes.
  typedef std::vector<std::tuple<
    int,         ///> id
    std::string, ///> label
    std::string, ///> name
    std::string, ///> config
    std::string, ///> tooltip
    bool,        ///> default value
    /// callback to handle click on box,
    /// if nullptr config item is changed
    std::function<void(wxCheckBox*)>>>
    checkboxes_t;

  /// Constructor.
  toolbar(
    frame*              frame,
    const data::window& data = data::window().style(wxAUI_TB_DEFAULT_STYLE));

  /// Adds a vector of checkbox controls to this toolbar.
  void add_checkboxes(const checkboxes_t& v, bool realize = true);

  /// Adds the standard checkboxes.
  /// This is a hex and a sync checkbox, and a process check box
  /// if you called prepare_output previously.
  void add_checkboxes_standard(bool realize = true);

  /// Adds a find ctrl, up and down arrows and checkboxes.
  /// The find ctrl allows you to find in a grid, stc
  /// or listview component on the specified frame, and
  /// is implemented as a ex-commandline.
  void add_find(bool realize = true);

  /// Adds the standard controls (e.g. file open).
  void add_standard(bool realize = true);

  /// Adds a vector of general toolbar items to this toolbar.
  /// If bitmap in wex::art is present the bitmap from the art is added.
  /// Returns false if one of the items could not be added.
  bool add_tool(const std::vector<data::toolbar_item>& v, bool realize = true);

  /// Sets checkbox state of checkboxes added using add_checkboxes.
  /// Returns true if checkbox was found.
  bool set_checkbox(
    /// name of checkbox to set
    const std::string& name,
    /// value
    bool value) const;

  /// overridden methods

  bool Destroy() override;

private:
  find_bar* m_find_bar;
  frame*    m_frame;

  std::vector<wxCheckBox*> m_checkboxes;
};
}; // namespace wex
