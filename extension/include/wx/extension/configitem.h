////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.h
// Purpose:   Declaration of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/slider.h> // for wxSL_HORIZONTAL
#include <wx/extension/item.h>

/// Callback for load or save data for user window.
typedef bool (*wxExUserWindowToConfig)(wxWindow* user, bool save);

#if wxUSE_GUI
/// Adds config support to wExItem.
class WXDLLIMPEXP_BASE wxExConfigItem : public wxExItem
{
public:
  /// Default constructor for a ITEM_EMPTY item.
  wxExConfigItem(const wxString& page = wxEmptyString)
    : wxExConfigItem(ITEM_EMPTY, 0, page, wxEmptyString) {;};

  /// Constructor for a ITEM_SPACER item.
  /// The size is the size for the spacer used.
  wxExConfigItem(int size, const wxString& page = wxEmptyString)
    : wxExConfigItem(ITEM_SPACER, size, page, wxEmptyString) {;};

  /// Constuctor for a ITEM_STATICLINE item.
  /// The orientation is wxHORIZONTAL or wxVERTICAL.
  wxExConfigItem(wxOrientation orientation, const wxString& page = wxEmptyString)
    : wxExConfigItem(ITEM_STATICLINE, orientation, page, wxEmptyString) {;};
    
  /// Constructor for a ITEM_STRING, ITEM_STC, ITEM_STATICTEXT, 
  /// or a ITEM_HYPERLINKCTRL item.
  wxExConfigItem(
    /// label for the window as on the dialog and in the config,
    /// might also contain the note after a tab for a command link button
    /// if the window supports it you can use a markup label
    const wxString& label,
    /// extra info, used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& info,
    const wxString& page = wxEmptyString,
    /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
    long style = 0,
    wxExItemType type = ITEM_STRING,
    bool is_required = false,
    /// will the label be displayed as a static text
    /// ignored for a static text
    bool add_label = true,
    int cols = -1)
    : wxExConfigItem(type, style, page, label, info, is_required, 
      (type != ITEM_STATICTEXT && 
       type != ITEM_HYPERLINKCTRL ? add_label: false), wxID_ANY, cols) {;};

  /// Constructor for a ITEM_SPINCTRL, ITEM_SPINCTRL_DOUBLE,
  /// ITEM_SPINCTRL_HEX or a ITEM_SLIDER item.
  wxExConfigItem(const wxString& label,
    double min, 
    double max,
    const wxString& page = wxEmptyString,
    wxExItemType type = ITEM_SPINCTRL,
    /// style for a ITEM_SLIDER item
    long style = wxSL_HORIZONTAL,
    double inc = 1,
    int cols = -1)
    : wxExConfigItem(type, style, page, label, 
      wxEmptyString, false, true, wxID_ANY, cols, 25, 1, 
      min, max, inc) {;};

  /// Constructor for a ITEM_CHECKLISTBOX_NONAME item. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExConfigItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices_bool,
    const wxString& page = wxEmptyString,
    long style = 0,
    int cols = -1)
    : wxExConfigItem(ITEM_CHECKLISTBOX_NONAME, style, page, "checklistbox_noname", 
      wxEmptyString, false, false, wxID_ANY, cols, 25, 1,
      0, 1, 1,
      std::map<long, const wxString>(),
      choices_bool) {;};

  /// Constructor for a ITEM_RADIOBOX, or a ITEM_CHECKLISTBOX item. 
  /// This checklistbox (not mutually exclusive choices)
  /// can be used to get/set individual bits in a long.
  /// A radiobox (mutually exclusive choices)
  /// should be used when a long value can have a short
  /// set of possible individual values.
  wxExConfigItem(const wxString& label,
    /// the map with values and text
    const std::map<long, const wxString> & choices,
    /// indicates whether to use a radiobox or a checklistbox.
    bool use_radiobox = true,
    const wxString& page = wxEmptyString,
    /// major dimension for the radiobox
    int majorDimension = 0,
    long style = wxRA_SPECIFY_COLS,
    int cols = -1)
    : wxExConfigItem(use_radiobox ? ITEM_RADIOBOX: ITEM_CHECKLISTBOX, style, page, label, 
      wxEmptyString, false, false, wxID_ANY, cols, 25, majorDimension, 
      0, 1, 1, 
      choices,
      std::set<wxString>()) {;};

  /// Constructor for a ITEM_USER item.
  wxExConfigItem(const wxString& label,
    /// the window (use default constructor for it)
    wxWindow* window,
    /// callback for window creation (required, useless without one)
    wxExUserWindowCreate create,
    /// callback for load and save to config
    /// default it has no relation to the config
    wxExUserWindowToConfig config = NULL,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    bool add_label = true,
    int cols = -1)
    : wxExConfigItem(ITEM_USER, 0, page, label, 
      wxEmptyString, is_required, add_label, wxID_ANY, cols, 25, 1, 
      0, 1, 1, 
      std::map<long, const wxString>(),
      std::set<wxString>(),
      window, create, config) {;};

  /// Constuctor for the other types (as ITEM_BUTTON item).
  wxExConfigItem(
    const wxString& label,
    wxExItemType type,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    /// the id as used by the window, see wxExFrame::OnCommandItemDialog, 
    int id = wxID_ANY,
    /// used by ITEM_COMBOBOX
    int max_items = 25,
    bool add_label = true,
    long style = 0,
    int cols = -1)
    : wxExConfigItem(type, style, page, label, wxEmptyString, is_required, 
        type == ITEM_BUTTON ||
        type == ITEM_CHECKBOX ||
        type == ITEM_COMMAND_LINK_BUTTON ||
        type == ITEM_TOGGLEBUTTON ? false: add_label, id, cols, max_items) {;};
    
  /// Loads or saves this item to the config.
  /// Returns true if the config was accessed, as not all
  /// config items associate with the config.
  virtual bool ToConfig(bool save) const override;
private:
  /// Delegate constructor.
  wxExConfigItem(wxExItemType type, long style,
    const wxString& page, const wxString& label, const wxString& info = wxEmptyString,
    bool is_required = false, bool add_label = false,
    int id = wxID_ANY, int cols = -1, int max_items = 25, int major_dimension = 1,
    double min = 0, double max = 1, double inc = 1,
    std::map<long, const wxString> choices = std::map<long, const wxString>(),
    std::set<wxString> choices_bool = std::set<wxString>(),
    wxWindow* window = NULL, wxExUserWindowCreate create = NULL, wxExUserWindowToConfig config = NULL)
    : wxExItem(type, style, page, label, wxEmptyString, info, is_required, add_label, id,
        cols, major_dimension, min, max, inc, 
        choices, choices_bool,
        window, create)
    , m_MaxItems(max_items)
    , m_UserWindowToConfig(config) {;};
  
  wxExUserWindowToConfig m_UserWindowToConfig;
  int m_MaxItems;
};
#endif // wxUSE_GUI
