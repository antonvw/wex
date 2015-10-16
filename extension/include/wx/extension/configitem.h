////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.h
// Purpose:   Declaration of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <wx/sizer.h> // for wxSizer, and wxSizerFlags
#include <wx/slider.h> // for wxSL_HORIZONTAL
#include <wx/string.h>
#include <wx/validate.h>

class wxFlexGridSizer;
class wxWindow;

#if wxUSE_GUI
/*! \file */
/// The config item types supported.
enum wxExConfigType
{
  /// Used for automatic testing only.
  CONFIG_ITEM_MIN,
  
  CONFIG_BUTTON,               ///< a wxButton item
  CONFIG_CHECKBOX,             ///< a wxCheckBox item (use ReadBool to retrieve value)
  CONFIG_CHECKLISTBOX,         ///< a wxCheckListBox item
  CONFIG_CHECKLISTBOX_NONAME,  ///< a wxCheckListBox item using boolean choices
  CONFIG_COLOUR,               ///< a wxColourPickerWidget item
  CONFIG_COMBOBOX,             ///< a wxComboBox item
  CONFIG_COMBOBOXDIR,          ///< a wxComboBox item with a browse button
  CONFIG_COMMAND_LINK_BUTTON,  ///< a wxCommandLinkButton button
  CONFIG_DIRPICKERCTRL,        ///< a wxDirPickerCtrl ctrl item
  CONFIG_EMPTY,                ///< an empty item
  CONFIG_FILEPICKERCTRL,       ///< a wxFilePickerCtrl ctrl item
  CONFIG_FLOAT,                ///< a wxTextCtrl item that only accepts a float (double)
  CONFIG_FONTPICKERCTRL,       ///< a wxFontPickerCtrl ctrl item
  CONFIG_HYPERLINKCTRL,        ///< a wxHyperlinkCtrl ctrl item
  CONFIG_INT,                  ///< a wxTextCtrl item that only accepts an integer (long)
  CONFIG_LISTVIEW_FOLDER,      ///< a wxExListViewFileName ctrl item (a list view standard file)
  CONFIG_RADIOBOX,             ///< a wxRadioBox item
  CONFIG_SLIDER,               ///< a wxSlider item
  CONFIG_SPACER,               ///< a spacer item
  CONFIG_SPINCTRL,             ///< a wxSpinCtrl item
  CONFIG_SPINCTRL_DOUBLE,      ///< a wxSpinCtrlDouble item
  CONFIG_SPINCTRL_HEX,         ///< a wxSpinCtrl hex item
  CONFIG_STATICLINE,           ///< a wxStaticLine item
  CONFIG_STATICTEXT,           ///< a wxStaticText item
  CONFIG_STC,                  ///< a wxExSTC ctrl item  
  CONFIG_STRING,               ///< a wxTextCtrl item
  CONFIG_TOGGLEBUTTON,         ///< a wxToggleButton item
  CONFIG_USER,                 ///< provide your own window

  /// Used for automatic testing only.
  CONFIG_ITEM_MAX
};

/// Callback for user window creation.
typedef void (*wxExUserWindowCreate)(wxWindow* user, wxWindow* parent, bool readonly);

/// Callback for load or save data for user window.
typedef bool (*wxExUserWindowToConfig)(wxWindow* user, bool save);

/// Container class for using with wxExConfigDialog.
/// - If you specify a page, then all items are placed on that page 
///   in a book ctrl on the config dialog.
///   You can specify the number of columns for the page after :
///   in the page name.
/// - If you specify add label, then the label is added as a label to
///   the item as well, otherwise the label is not added, and only used
///   for loading and saving from config.
/// - If you use the default for cols, then the number of cols used
///   is determined by the config dialog, otherwise this number is used.
/// - Default whether the item row is growable is determined
///   by the kind of item. You can ovverride this using SetRowGrowable.
class WXDLLIMPEXP_BASE wxExConfigItem
{
public:
  /// Default constructor for a CONFIG_EMPTY item.
  wxExConfigItem(const wxString& page = wxEmptyString)
    : wxExConfigItem(CONFIG_EMPTY, 0, page, wxEmptyString) {;};

  /// Constructor for a CONFIG_SPACER item.
  /// The size is the size for the spacer used.
  wxExConfigItem(int size, const wxString& page = wxEmptyString)
    : wxExConfigItem(CONFIG_SPACER, size, page, wxEmptyString) {;};

  /// Constuctor for a CONFIG_STATICLINE item.
  /// The orientation is wxHORIZONTAL or wxVERTICAL.
  wxExConfigItem(wxOrientation orientation, const wxString& page = wxEmptyString)
    : wxExConfigItem(CONFIG_STATICLINE, orientation, page, wxEmptyString) {;};
    
  /// Constructor for a CONFIG_STRING, CONFIG_STC, CONFIG_STATICTEXT, 
  /// or a CONFIG_HYPERLINKCTRL item.
  wxExConfigItem(
    /// label for the window as on the dialog and in the config,
    /// - might also contain the note after a tab for a command link button
    /// - if the window supports it you can use a markup label
    const wxString& label,
    /// extra info, used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& info,
    const wxString& page = wxEmptyString,
    /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
    long style = 0,
    wxExConfigType type = CONFIG_STRING,
    bool is_required = false,
    /// will the label be displayed as a static text
    /// ignored for a static text
    bool add_label = true,
    int cols = -1)
    : wxExConfigItem(type, style, page, label, info, is_required, 
      (type != CONFIG_STATICTEXT && 
       type != CONFIG_HYPERLINKCTRL ? add_label: false), wxID_ANY, cols) {;};

  /// Constructor for a CONFIG_SPINCTRL, CONFIG_SPINCTRL_DOUBLE,
  /// CONFIG_SPINCTRL_HEX or a CONFIG_SLIDER item.
  wxExConfigItem(const wxString& label,
    double min, 
    double max,
    const wxString& page = wxEmptyString,
    wxExConfigType type = CONFIG_SPINCTRL,
    /// style for a CONFIG_SLIDER item
    long style = wxSL_HORIZONTAL,
    double inc = 1,
    int cols = -1)
    : wxExConfigItem(type, style, page, label, wxEmptyString, false, true, wxID_ANY, cols, 25, 1, min, max, inc) {;};

  /// Constructor for a CONFIG_CHECKLISTBOX_NONAME item. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExConfigItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices,
    const wxString& page = wxEmptyString,
    int cols = -1)
    : wxExConfigItem(CONFIG_CHECKLISTBOX_NONAME, 0, page, "checklistbox_noname", wxEmptyString, false, false, wxID_ANY, cols)
  {
    m_ChoicesBool = choices;
  }

  /// Constructor for a CONFIG_RADIOBOX, or a CONFIG_CHECKLISTBOX item. 
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
    : wxExConfigItem(use_radiobox ? CONFIG_RADIOBOX: CONFIG_CHECKLISTBOX, style, page, label, wxEmptyString, false, false, wxID_ANY, cols, 25, majorDimension)
  {
    m_Choices = choices;
  }

  /// Constructor for a CONFIG_USER item.
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
    : wxExConfigItem(CONFIG_USER, 0, page, label, wxEmptyString, is_required, add_label, wxID_ANY, cols, 25, 1, 0, 0, 0, window, create, config) {;};

  /// Constuctor for the other types (as CONFIG_BUTTON item).
  wxExConfigItem(
    const wxString& label,
    wxExConfigType type,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    /// the id as used by the window, see wxExFrame::OnCommandConfigDialog, 
    int id = wxID_ANY,
    /// used by CONFIG_COMBOBOX
    int max_items = 25,
    bool add_label = true,
    int cols = -1)
    : wxExConfigItem(type, 0, page, label, wxEmptyString, is_required, 
        type == CONFIG_BUTTON ||
        type == CONFIG_CHECKBOX ||
        type == CONFIG_COMMAND_LINK_BUTTON ||
        type == CONFIG_TOGGLEBUTTON ? false: add_label, id, cols, max_items) {;};
    
  /// Gets the number of columns for the current page.
  int GetColumns() const {return m_PageCols;};

  /// Gets is required.
  bool GetIsRequired() const {return m_IsRequired;};

  /// Gets the label.
  const wxString& GetLabel() const {return m_Label;};

  /// Gets the page.
  const wxString& GetPage() const {return m_Page;};

  /// Gets the type.
  wxExConfigType GetType() const {return m_Type;};

  /// Gets the window (first call Layout, to create it, otherwise it is NULL).
  wxWindow* GetWindow() const {return m_Window;};
  
  /// Is this item allowed to be expanded on a row.
  bool IsRowGrowable() const {return m_IsRowGrowable;};

  /// Layouts this item (creates the window) on the specified sizer, and fills it
  /// with config value (calls ToConfig).
  /// It returns the flex grid sizer that was used for creating the item sizer.
  /// Or it returns NULL if no flex grid sizer was used.
  wxFlexGridSizer* Layout(
    /// the parent
    wxWindow* parent, 
    /// the sizer
    wxSizer* sizer,
    /// specify the item will be readonly, it will not be changeable
    /// if underlying control supports this
    bool readonly = false,
    /// specify the sizer for creating the item, or NULL,
    /// than a new one is created
    wxFlexGridSizer* fgz = NULL);
    
  /// Sets this item to be growable.
  void SetRowGrowable(bool value) {m_IsRowGrowable = value;};
  
  /// Sets the validator to be used by the config item, if appropriate.
  /// Default a normal wxDefaultValidator is used, except for CONFIG_INT,
  /// that uses a wxTextValidator with wxFILTER_NUMERIC.
  void SetValidator(wxValidator* validator) {m_Validator = validator;};
    
  /// Loads or saves this item to the config.
  /// Returns true if the config was accessed, as not all
  /// config items associate with the config.
  bool ToConfig(bool save) const;
private:
  /// Delegate constructor.
  wxExConfigItem(wxExConfigType type, long style,
    const wxString& page, const wxString& label, const wxString& info = wxEmptyString,
    bool is_required = false, bool add_label = false,
    int id = wxID_ANY, int cols = -1, int max_items = 25, int major_dimension = 1,
    double min = 0, double max = 1, double inc = 1,
    wxWindow* window = NULL, wxExUserWindowCreate create = NULL, wxExUserWindowToConfig config = NULL);
  
  wxFlexGridSizer* AddBrowseButton(wxSizer* sizer) const;
  void AddStaticText(wxSizer* sizer) const;
  void CreateWindow(wxWindow* parent, bool readonly);

  bool m_AddLabel;
  bool m_IsRequired;
  bool m_IsRowGrowable;

  int m_Cols;
  int m_Id;
  int m_MajorDimension;
  int m_MaxItems;
  int m_PageCols;
  
  double m_Min;
  double m_Max;
  double m_Inc;

  wxString m_Label;
  wxString m_Info;
  wxString m_Page;

  long m_Style;

  std::map<long, const wxString> m_Choices;
  std::set<wxString> m_ChoicesBool;

  wxExUserWindowCreate m_UserWindowCreate;
  wxExUserWindowToConfig m_UserWindowToConfig;
  
  wxExConfigType m_Type;
  wxSizerFlags m_SizerFlags;
  wxValidator* m_Validator;
  wxWindow* m_Window;
};
#endif // wxUSE_GUI
