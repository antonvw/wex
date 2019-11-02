////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.h
// Purpose:   Declaration of wex::toolbar classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/aui/auibar.h> 
#include <wex/window-data.h> 

namespace wex
{
  class managed_frame;

  /// Offers a toolbar together with stock art.
  /// Default no controls are added, you have to call 
  /// one of the add_ methods to do that.
  class toolbar : public wxAuiToolBar
  {
  public:
    /// Type for adding checkboxes.
    typedef std::vector<std::tuple<
        int,          ///> id
        std::string,  ///> label
        std::string,  ///> name
        std::string,  ///> config
        std::string,  ///> tooltip
        bool,         ///> default value
        /// callback to handle click on box,
        /// if nullptr config item is changed
        std::function<void(wxCheckBox*)> >>       
      checkboxes_t;
    
    /// Constructor.
    toolbar(managed_frame* frame, 
      const window_data& data = window_data().style(wxAUI_TB_DEFAULT_STYLE));

    /// Adds a vector of checkbox controls to this toolbar.
    void add_checkboxes(
      const checkboxes_t& v,
      bool realize = true);
    
    /// Adds the standard checkboxes.
    /// This is a hex and a sync checkbox, and a process check box
    /// if you called prepare_output previously.
    void add_checkboxes_standard(bool realize = true);

    /// Adds a find textctrl, up and down arrows
    /// and checkboxes.
    /// The find textctrl allows you to find in a grid, stc
    /// or listview component on the specified frame.
    void add_find(bool realize = true);

    /// Adds the standard controls (e.g. file open).
    void add_standard(bool realize = true);
    
    /// Adds automatic naming (for stock menu id's) and 
    /// art id for toolbar normal items.
    wxAuiToolBarItem* add_tool(int toolId,
      const std::string& label = std::string(),
      const wxBitmap& bitmap = wxNullBitmap,
      const std::string& shortHelp = std::string(),
      wxItemKind kind = wxITEM_NORMAL);

    /// Sets checkbox state of checkboxes added using add_checkboxes.
    /// Returns true if checkbox was found.
    bool set_checkbox(
      /// name of checkbox to set
      const std::string& name, 
      /// value
      bool value) const;
  private:
    managed_frame* m_frame;
    std::vector<wxCheckBox*> m_checkboxes;
  };
};
