////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.h
// Purpose:   Declaration of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXCONFIGITEM_H
#define _EXCONFIGITEM_H

#include <map>
#include <set>
#include <wx/sizer.h> // for wxSizer, and wxSizerFlags
#include <wx/slider.h> // for wxSL_HORIZONTAL
#include <wx/string.h>
#include <wx/valtext.h>

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

  /// a button item
  CONFIG_BUTTON,
  
  /// a checkbox item (use ReadBool to retrieve value)
  CONFIG_CHECKBOX,

  /// a checklistbox item
  CONFIG_CHECKLISTBOX,

  /// a checklistbox item
  CONFIG_CHECKLISTBOX_NONAME,
  
  /// a colour button item
  CONFIG_COLOUR,
  
  /// a combobox item
  CONFIG_COMBOBOX,

  /// a combobox item with a browse button
  CONFIG_COMBOBOXDIR,

  /// a commandlink button
  CONFIG_COMMAND_LINK_BUTTON,

  /// a dirpicker ctrl item
  CONFIG_DIRPICKERCTRL,

  /// an empty item
  CONFIG_EMPTY,

  /// a filepicker ctrl item
  CONFIG_FILEPICKERCTRL,

  /// a textctrl item that only accepts a float (double)
  CONFIG_FLOAT,
  
  /// a fontpicker ctrl item
  CONFIG_FONTPICKERCTRL,

  /// a hyperlink ctrl item
  CONFIG_HYPERLINKCTRL,

  /// a textctrl item that only accepts an integer (long)
  CONFIG_INT,
  
  /// a listview ctrl item (a list view standard file)
  CONFIG_LISTVIEW_FOLDER,

  /// a radiobox item
  CONFIG_RADIOBOX,

  /// a slider item
  CONFIG_SLIDER,

  /// a spacer item
  CONFIG_SPACER,

  /// a spinctrl item
  CONFIG_SPINCTRL,

  /// a spinctrl double item
  CONFIG_SPINCTRL_DOUBLE,

  /// a spinctrl hex item
  CONFIG_SPINCTRL_HEX,

  /// a static line item (default horizontal)
  CONFIG_STATICLINE,

  /// a static text item
  CONFIG_STATICTEXT,

  /// a textctrl item
  CONFIG_STRING,

  /// a styled text ctrl item  
  CONFIG_STC,

  /// a toggle button item
  CONFIG_TOGGLEBUTTON,
  
  /// provide your own window
  CONFIG_USER,

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
  /// Default constructor.
  wxExConfigItem();
  
  /// Constructor for spacer config item.
  /// The size is the size for the spacer used.
  wxExConfigItem(int size);
  
  /// Constuctor for a static horizontal or vertical line.
  /// Or for an empty config item on specified page.
  wxExConfigItem(
    /// style: wxLI_HORIZONTAL or wxLI_VERTICAL
    /// or size for a empty config item
    long style, 
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// type
    wxExConfigType type = CONFIG_STATICLINE);
    
  /// Constuctor for most types.
  wxExConfigItem(
    /// label for the window as on the dialog and in the config,
    /// - might also contain the note after a tab for a command link button
    /// - if the window supports it you can use a markup label
    const wxString& label,
    /// type
    wxExConfigType type,
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// is this item required
    bool is_required = false,
    /// the id as used by the window, 
    /// when using for a combobox dir, use id < wxID_LOWEST
    /// accessible using GetWindow()->GetId()
    int id = wxID_ANY,
    /// used by CONFIG_COMBOBOX
    int max_items = 25,
    /// will the label be displayed as a static text
    bool add_label = true,
    /// the number of cols
    int cols = -1);
    
  /// Constructor for a user window.
  wxExConfigItem(
    /// label for the window as on the dialog and in the config
    const wxString& label,
    /// the window (use default constructor for it)
    wxWindow* window,
    /// callback for window creation (required, useless without one)
    wxExUserWindowCreate user,
    /// callback for load and save to config
    /// default it has no relation to the config
    wxExUserWindowToConfig cfg = NULL,
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// is this control required
    bool is_required = false,
    /// will the label be displayed as a static text
    bool add_label = true,
    /// number of cols for this control
    int cols = -1);

  /// Constructor for a string, a hyperlink ctrl, a static text or a STC.
  /// The extra style argument is the style for the control used
  /// (e.g. wxTE_MULTILINE or wxTE_PASSWORD).
  wxExConfigItem(
    /// label for the control as on the dialog and in the config
    const wxString& label,
    /// used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& value = wxEmptyString,
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// the style
    long style = 0,
    /// the type
    wxExConfigType type = CONFIG_STRING,
    /// is this item required
    bool is_required = false,
    /// ignored for a static text
    bool add_label = true,
    /// number of cols for this control
    int cols = -1);

  /// Constructor for a spin ctrl, a spin ctrl double,
  /// a spin ctrl hex, or a slider.
  wxExConfigItem(
    /// label for the control as on the dialog and in the config
    const wxString& label,
    /// minimum value
    double min, 
    /// maximum value
    double max,
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// type
    wxExConfigType type = CONFIG_SPINCTRL,
    /// style
    long style = wxSL_HORIZONTAL,
    /// incrment value
    double inc = 1,
    /// number of cols for this control
    int cols = -1);

  /// Constructor for a checklistbox without a label. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExConfigItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices,
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// number of cols for this control
    int cols = -1);

  /// Constructor for a radiobox or a checklistbox. 
  /// This checklistbox (not mutually exclusive choices)
  /// can be used to get/set individual bits in a long.
  /// A radiobox (mutually exclusive choices)
  /// should be used when a long value can have a short
  /// set of possible individual values.
  wxExConfigItem(
    /// label for the control as on the dialog and in the config
    const wxString& label,
    /// the map with values and text
    const std::map<long, const wxString> & choices,
    /// indicates whether to use a radiobox or a checklistbox.
    bool use_radiobox = true,
    /// page on notebook
    const wxString& page = wxEmptyString,
    /// major dimension for the radiobox
    int majorDimension = 0,
    /// style for the radiobox
    long style = wxRA_SPECIFY_COLS,
    /// number of columns for this control
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
  void Init(const wxString& page, int cols);
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
#endif
