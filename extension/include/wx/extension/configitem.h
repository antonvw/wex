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

class wxCheckListBox;
class wxFlexGridSizer;
class wxWindow;

#if wxUSE_GUI
/*! \file */
/// The config item types supported.
enum wxExConfigType
{
  /// Used for automatic testing only.
  CONFIG_ITEM_MIN,
  
  CONFIG_BUTTON,               /// a wxButton item
  CONFIG_CHECKBOX,             /// a wxCheckBox item (use ReadBool to retrieve value)
  CONFIG_CHECKLISTBOX,         /// a wxCheckListBox item
  CONFIG_CHECKLISTBOX_NONAME,  /// a wxCheckListBox item using boolean choices
  CONFIG_COLOUR,               /// a wxColourPickerWidget item
  CONFIG_COMBOBOX,             /// a wxComboBox item
  CONFIG_COMBOBOXDIR,          /// a wxComboBox item with a browse button
  CONFIG_COMMAND_LINK_BUTTON,  /// a wxCommandLinkButton button
  CONFIG_DIRPICKERCTRL,        /// a wxDirPickerCtrl ctrl item
  CONFIG_EMPTY,                /// an empty item
  CONFIG_FILEPICKERCTRL,       /// a wxFilePickerCtrl ctrl item
  CONFIG_FLOAT,                /// a wxTextCtrl item that only accepts a float (double)
  CONFIG_FONTPICKERCTRL,       /// a wxFontPickerCtrl ctrl item
  CONFIG_HYPERLINKCTRL,        /// a wxHyperlinkCtrl ctrl item
  CONFIG_INT,                  /// a wxTextCtrl item that only accepts an integer (long)
  CONFIG_LISTVIEW_FOLDER,      /// a wxExListViewFileName ctrl item (a list view standard file)
  CONFIG_RADIOBOX,             /// a wxRadioBox item
  CONFIG_SLIDER,               /// a wxSlider item
  CONFIG_SPACER,               /// a spacer item
  CONFIG_SPINCTRL,             /// a wxSpinCtrl item
  CONFIG_SPINCTRL_DOUBLE,      /// a wxSpinCtrlDouble item
  CONFIG_SPINCTRL_HEX,         /// a wxSpinCtrl hex item
  CONFIG_STATICLINE,           /// a wxStaticLine item (default horizontal)
  CONFIG_STATICTEXT,           /// a wxStaticText item
  CONFIG_STC,                  /// a wxExSTC ctrl item  
  CONFIG_STRING,               /// a wxTextCtrl item
  CONFIG_TOGGLEBUTTON,         /// a wxToggleButton item
  CONFIG_USER,                 /// provide your own window

  /// Used for automatic testing only.
  CONFIG_ITEM_MAX
};

/// Callback for user window creation.
typedef void (*wxExUserWindowCreate)(
  wxWindow* user, 
  wxWindow* parent, 
  bool readonly);

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
  /// Default constructor (for empty config item).
  wxExConfigItem(
    /// type
    wxExConfigType type = CONFIG_EMPTY,
    /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
    /// size for a empty config item
    long style = 0,
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// label for the window as on the dialog and in the config,
    /// - might also contain the note after a tab for a command link button
    /// - if the window supports it you can use a markup label
    const wxString& label = wxEmptyString,
    /// used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& value = wxEmptyString,
    /// is this item required
    bool is_required = false,
    /// will the label be displayed as a static text
    /// ignored for a static text
    bool add_label = true,
    /// the id as used by the window, see wxExFrame::OnCommandConfigDialog, 
    int id = wxID_ANY,
    /// the number of cols
    int cols = -1,
    /// used by CONFIG_COMBOBOX
    int max_items = 25,
    /// major dimension for the radiobox
    int major_dimension = 1,
    /// minimum value
    double min = 0,
    /// maximum value
    double max = 0,
    /// incrment value
    double inc = 0,
    /// the window (use default constructor for it)
    wxWindow* window = NULL,
    /// callback for window creation (required, useless without one)
    wxExUserWindowCreate create = NULL,
    /// callback for load and save to config
    /// default it has no relation to the config
    wxExUserWindowToConfig config = NULL,
    wxValidator* validator = NULL);
  
  /// Constructor for spacer config item.
  /// The size is the size for the spacer used.
  wxExConfigItem(int size);
  
  /// Constuctor for a static horizontal or vertical line.
  wxExConfigItem(
    /// style: wxLI_HORIZONTAL or wxLI_VERTICAL
    long style, 
    const wxString& page = wxEmptyString);
    
  /// Constuctor for most types.
  wxExConfigItem(
    const wxString& label,
    wxExConfigType type,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    int id = wxID_ANY,
    int max_items = 25,
    bool add_label = true,
    int cols = -1);
    
  /// Constructor for a user window.
  wxExConfigItem(
    const wxString& label,
    wxWindow* window,
    wxExUserWindowCreate user,
    wxExUserWindowToConfig cfg = NULL,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    bool add_label = true,
    int cols = -1);

  /// Constructor for a string, a hyperlink ctrl, a static text or a STC.
  wxExConfigItem(
    const wxString& label,
    const wxString& value = wxEmptyString,
    const wxString& page = wxEmptyString,
    long style = 0,
    wxExConfigType type = CONFIG_STRING,
    bool is_required = false,
    bool add_label = true,
    int cols = -1);

  /// Constructor for a spin ctrl, a spin ctrl double,
  /// a spin ctrl hex, or a slider.
  wxExConfigItem(
    const wxString& label,
    double min, 
    double max,
    const wxString& page = wxEmptyString,
    wxExConfigType type = CONFIG_SPINCTRL,
    long style = wxSL_HORIZONTAL,
    double inc = 1,
    int cols = -1);

  /// Constructor for a checklistbox without a label. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExConfigItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices,
    const wxString& page = wxEmptyString,
    int cols = -1);

  /// Constructor for a radiobox or a checklistbox. 
  /// This checklistbox (not mutually exclusive choices)
  /// can be used to get/set individual bits in a long.
  /// A radiobox (mutually exclusive choices)
  /// should be used when a long value can have a short
  /// set of possible individual values.
  wxExConfigItem(
    const wxString& label,
    /// the map with values and text
    const std::map<long, const wxString> & choices,
    /// indicates whether to use a radiobox or a checklistbox.
    bool use_radiobox = true,
    const wxString& page = wxEmptyString,
    int majorDimension = 0,
    long style = wxRA_SPECIFY_COLS,
    int cols = -1);

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

  /// Creates the window,
  /// lays out this item on the specified sizer, and fills it
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
  wxFlexGridSizer* AddBrowseButton(wxSizer* sizer) const;
  void AddStaticText(wxSizer* sizer) const;
  void CreateWindow(wxWindow* parent, bool readonly);
  bool Get(const wxString& field, wxCheckListBox* clb, int item) const;

  // The members are allowed to be const using
  // MS Visual Studio 2010, not using gcc, so
  // removed again (operator= seems to be used).
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
  wxString m_Page;
  wxString m_Default; // used by hyperlink as default web address

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
