////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.h
// Purpose:   Declaration of wex::toolbar classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/aui/auibar.h> 
#include <wex/window-data.h> 

namespace wex
{
  class managed_frame;

  /// Offers a toolbar together with stock art.
  /// Default no controls are added, you have to call add_controls to do that.
  class toolbar : public wxAuiToolBar
  {
  public:
    /// Constructor.
    toolbar(managed_frame* frame, 
      const window_data& data = window_data().style(wxAUI_TB_DEFAULT_STYLE));

    /// Adds automatic naming (for stock menu id's) and 
    /// art id for toolbar normal items.
    wxAuiToolBarItem* add_tool(int toolId,
      const wxString& label = wxEmptyString,
      const wxBitmap& bitmap = wxNullBitmap,
      const wxString& shortHelp = wxEmptyString,
      wxItemKind kind = wxITEM_NORMAL);

    /// Adds the standard controls.
    /// This adds a file open, save and print and find control.
    void add_controls(bool realize = true);
    
    /// Returns the frame.
    managed_frame* frame() {return m_Frame;};
  private:
    managed_frame* m_Frame;
  };

  /// Offers a find toolbar, containing a find ctrl, up and down arrows
  /// and checkboxes.
  /// The find ctrl allows you to find in an stc
  /// component on the specified frame.
  class find_toolbar : public toolbar
  {
  public:
    /// Constructor.
    find_toolbar(managed_frame* frame, 
      const window_data& data = window_data().style(wxAUI_TB_DEFAULT_STYLE));
  };

  /// Offers a options toolbar, containing checkboxes.
  class options_toolbar : public toolbar
  {
  public:
    /// Constructor.
    options_toolbar(managed_frame* frame, 
      const window_data& data = window_data().style(wxAUI_TB_DEFAULT_STYLE));
    
    /// Adds the standard checkboxes.
    /// This is a hex and a sync checkbox, and a process check box
    /// if you called prepare_output previously.
    void add_controls(bool realize = true);
    
    /// Updates checkbox state.
    /// Returns true if checkbox was found.
    bool update(const wxString& name, bool show);
  private:
    std::vector<wxCheckBox*> m_CheckBoxes;
  };
};
