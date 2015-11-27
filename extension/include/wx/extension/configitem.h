////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.h
// Purpose:   Declaration of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>
#include <vector>
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
  wxExConfigItem() : wxExConfigItem(ITEM_EMPTY, 0) {;};

  /// Constructor for a ITEM_SPACER item.
  /// The size is the size for the spacer used.
  wxExConfigItem(int size, const wxString& page = wxEmptyString)
    : wxExConfigItem(ITEM_SPACER, size, page) {;};

  /// Constuctor for a ITEM_STATICLINE item.
  /// The orientation is wxHORIZONTAL or wxVERTICAL.
  wxExConfigItem(wxOrientation orientation, const wxString& page = wxEmptyString)
    : wxExConfigItem(ITEM_STATICLINE, orientation, page) {;};
    
  /// Constructor for a ITEM_STRING, ITEM_STC, ITEM_STATICTEXT, 
  /// or a ITEM_HYPERLINKCTRL item.
  wxExConfigItem(
    /// label for the window as on the dialog and in the config,
    /// might also contain the note after a tab for a command link button
    /// if the window supports it you can use a markup label
    const wxString& label,
    /// used as default for a hyperlink ctrl, or as lexer for STC
    /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
    const wxString& value = wxEmptyString,
    const wxString& page = wxEmptyString,
    long style = 0,
    wxExItemType type = ITEM_STRING,
    bool is_required = false,
    /// will the label be displayed as a static text
    /// (ignored for a static text item itself)
    wxExLabelType label_type = LABEL_LEFT)
    : wxExConfigItem(type, style, page, label, is_required, 
      (type != ITEM_STATICTEXT && 
       type != ITEM_HYPERLINKCTRL ? label_type: LABEL_NONE), wxID_ANY, 25, 1, 0, 1, 1, value) {;};

  /// Constructor for a ITEM_SPINCTRL or a ITEM_SLIDER item.
  wxExConfigItem(const wxString& label,
    int min, 
    int max,
    const wxString& page = wxEmptyString,
    wxExItemType type = ITEM_SPINCTRL,
    /// style for a ITEM_SLIDER item
    long style = wxSL_HORIZONTAL)
    : wxExConfigItem(type, style, page, label, 
      false, LABEL_LEFT, wxID_ANY, 25, 1, min, max, 1, min) {;};

  /// Constructor for a ITEM_SPINCTRL_DOUBLE item.
  wxExConfigItem(const wxString& label,
    double min, 
    double max,
    const wxString& page = wxEmptyString,
    double inc = 1)
    : wxExConfigItem(ITEM_SPINCTRL_DOUBLE , 0, page, label, 
      false, LABEL_LEFT, wxID_ANY, 25, 1, min, max, inc, min) {;};

  /// Constructor for a ITEM_CHECKLISTBOX_BOOL item. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExConfigItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices,
    const wxString& page = wxEmptyString,
    long style = 0)
    : wxExConfigItem(ITEM_CHECKLISTBOX_BOOL, style, page, "checklistbox_noname", 
      false, LABEL_NONE, wxID_ANY, 25, 1, 0, 1, 1, choices) {;};

  /// Constructor for a ITEM_RADIOBOX, or a ITEM_CHECKLISTBOX_BIT item. 
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
    long style = wxRA_SPECIFY_COLS)
    : wxExConfigItem(use_radiobox ? ITEM_RADIOBOX: ITEM_CHECKLISTBOX_BIT, style, page, label, 
      false, LABEL_NONE, wxID_ANY, 25, majorDimension, 0, 1, 1, choices) {;};

  /// Constructor for a ITEM_USER item.
  wxExConfigItem(const wxString& label,
    /// the window (use default constructor for it)
    wxWindow* window,
    /// callback for window creation (required, useless without one)
    wxExUserWindowCreate create,
    /// callback for load and save to config
    /// default it has no relation to the config
    wxExUserWindowToConfig config = nullptr,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    wxExLabelType label_type = LABEL_LEFT)
    : wxExConfigItem(ITEM_USER, 0, page, label, 
      is_required, label_type, wxID_ANY, 25, 1, 0, 1, 1, wxAny(), window, create, config) {;};

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
    wxExLabelType label_type = LABEL_LEFT,
    long style = 0)
    : wxExConfigItem(type, style, page, label, is_required, 
        type == ITEM_BUTTON ||
        type == ITEM_CHECKBOX ||
        type == ITEM_COMMAND_LINK_BUTTON ||
        type == ITEM_TOGGLEBUTTON ? LABEL_NONE: label_type, id, max_items) {;};
    
  /// Layouts this item and calls ToConfig.
  virtual wxFlexGridSizer* Layout(
    wxWindow* parent, 
    wxSizer* sizer,
    bool readonly = false,
    wxFlexGridSizer* fgz = nullptr) override;

  /// Loads or saves this item to the config.
  /// Returns true if the config was accessed, as not all
  /// config items associate with the config.
  bool ToConfig(bool save) const;
private:
  /// Delegate constructor.
  wxExConfigItem(wxExItemType type, long style,
    const wxString& page = wxEmptyString, const wxString& label = wxEmptyString,
    bool is_required = false, wxExLabelType label_type = LABEL_NONE,
    int id = wxID_ANY, int max_items = 25, int major_dimension = 1,
    const wxAny& min = 0, const wxAny& max = 1, const wxAny& inc = 1, const wxAny& initial = wxAny(),
    wxWindow* window = nullptr, wxExUserWindowCreate create = nullptr, wxExUserWindowToConfig config = nullptr)
    : wxExItem(type, style, page, label, initial, is_required, label_type, id,
        major_dimension, min, max, inc, window, create)
    , m_MaxItems(max_items)
    , m_UserWindowToConfig(config) {;};
  
  wxExUserWindowToConfig m_UserWindowToConfig;
  int m_MaxItems;
};

class wxConfigBase;

/// Support class for keeping defaults in the config.
class WXDLLIMPEXP_BASE wxExConfigDefaults
{
public:
  /// Constructor, records default values,
  /// if not yet in the config.
  wxExConfigDefaults(
    /// supply name, item type, and default value
    const std::vector<std::tuple<wxString, wxExItemType, wxAny>> & items);
  
  /// Destructor, stops recording.
 ~wxExConfigDefaults();
  
  /// Access to config.
  wxConfigBase* Get() {return m_Config;};
private:
  wxConfigBase* m_Config;
};
#endif // wxUSE_GUI
