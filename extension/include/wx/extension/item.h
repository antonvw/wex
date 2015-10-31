////////////////////////////////////////////////////////////////////////////////
// Name:      item.h
// Purpose:   Declaration of wxExItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <wx/any.h>
#include <wx/sizer.h> // for wxSizer, and wxSizerFlags
#include <wx/string.h>
#include <wx/validate.h>

class wxFlexGridSizer;
class wxWindow;

#if wxUSE_GUI
/*! \file */
/// The item types supported.
enum wxExItemType
{
  /// Used for automatic testing only.
  ITEM_ITEM_MIN,
  
  ITEM_BUTTON,               ///< a wxButton item
  ITEM_CHECKBOX,             ///< a wxCheckBox item
  ITEM_CHECKLISTBOX_BIT,     ///< a wxCheckListBox item to set individual bits in a long
  ITEM_CHECKLISTBOX_BOOL,    ///< a wxCheckListBox item using boolean choices
  ITEM_COLOUR,               ///< a wxColourPickerWidget item
  ITEM_COMBOBOX,             ///< a wxComboBox item
  ITEM_COMBOBOXDIR,          ///< a wxComboBox item with a browse button
  ITEM_COMMAND_LINK_BUTTON,  ///< a wxCommandLinkButton button
  ITEM_DIRPICKERCTRL,        ///< a wxDirPickerCtrl ctrl item
  ITEM_EMPTY,                ///< an empty item
  ITEM_FILEPICKERCTRL,       ///< a wxFilePickerCtrl ctrl item
  ITEM_FLOAT,                ///< a wxTextCtrl item that only accepts a float (double)
  ITEM_FONTPICKERCTRL,       ///< a wxFontPickerCtrl ctrl item
  ITEM_HYPERLINKCTRL,        ///< a wxHyperlinkCtrl ctrl item
  ITEM_INT,                  ///< a wxTextCtrl item that only accepts an integer (long)
  ITEM_LISTVIEW_FOLDER,      ///< a wxExListViewFileName ctrl item (a list view standard file)
  ITEM_RADIOBOX,             ///< a wxRadioBox item
  ITEM_SLIDER,               ///< a wxSlider item
  ITEM_SPACER,               ///< a spacer item
  ITEM_SPINCTRL,             ///< a wxSpinCtrl item
  ITEM_SPINCTRL_DOUBLE,      ///< a wxSpinCtrlDouble item
  ITEM_SPINCTRL_HEX,         ///< a wxSpinCtrl hex item
  ITEM_STATICLINE,           ///< a wxStaticLine item
  ITEM_STATICTEXT,           ///< a wxStaticText item
  ITEM_STC,                  ///< a wxExSTC ctrl item  
  ITEM_STRING,               ///< a wxTextCtrl item
  ITEM_TOGGLEBUTTON,         ///< a wxToggleButton item
  ITEM_USER,                 ///< provide your own window

  /// Used for automatic testing only.
  ITEM_ITEM_MAX
};

/// Callback for user window creation.
typedef void (*wxExUserWindowCreate)(wxWindow* user, wxWindow* parent, bool readonly);

/// Container class for using with wxExItemDialog.
class WXDLLIMPEXP_BASE wxExItem
{
public:
  /// Default constructor for a ITEM_EMPTY item.
  wxExItem() : wxExItem(ITEM_EMPTY, 0) {;};

  /// Constructor for a ITEM_SPACER item.
  /// The size is the size for the spacer used.
  wxExItem(int size, const wxString& page = wxEmptyString)
    : wxExItem(ITEM_SPACER, size, page) {;};

  /// Constuctor for a ITEM_STATICLINE item.
  /// The orientation is wxHORIZONTAL or wxVERTICAL.
  wxExItem(wxOrientation orientation, const wxString& page = wxEmptyString)
    : wxExItem(ITEM_STATICLINE, orientation, page) {;};
    
  /// Constructor for a ITEM_STRING, ITEM_STC, ITEM_STATICTEXT, 
  /// or a ITEM_HYPERLINKCTRL item.
  wxExItem(
    /// label for the window as on the dialog,
    /// might also contain the note after a tab for a command link button
    /// if the window supports it you can use a markup label
    const wxString& label,
    /// initial value
    const wxString& value = wxEmptyString,
    /// extra info, used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& info = wxEmptyString,
    const wxString& page = wxEmptyString,
    /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
    long style = 0,
    wxExItemType type = ITEM_STRING,
    bool is_required = false,
    /// will the label be displayed as a static text
    /// ignored for a static text
    bool add_label = true,
    int cols = -1)
    : wxExItem(type, style, page, label, value, info, is_required, 
      (type != ITEM_STATICTEXT && 
       type != ITEM_HYPERLINKCTRL ? add_label: false), wxID_ANY, cols) {;};

  /// Constructor for a ITEM_SPINCTRL, ITEM_SPINCTRL_DOUBLE,
  /// ITEM_SPINCTRL_HEX or a ITEM_SLIDER item.
  wxExItem(const wxString& label,
    double min, 
    double max,
    const wxString& page = wxEmptyString,
    wxExItemType type = ITEM_SPINCTRL,
    /// style for a ITEM_SLIDER item
    long style = wxSL_HORIZONTAL,
    double inc = 1,
    int cols = -1)
    : wxExItem(type, style, page, label, wxEmptyString,
      wxEmptyString, false, true, wxID_ANY, cols, 1, 
      min, max, inc) {;};

  /// Constructor for a ITEM_CHECKLISTBOX_BOOL item. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices_bool,
    const wxString& page = wxEmptyString,
    long style = 0,
    int cols = -1)
    : wxExItem(ITEM_CHECKLISTBOX_BOOL, style, page, "checklistbox_bool", wxEmptyString,
      wxEmptyString, false, false, wxID_ANY, cols, 1,
      0, 1, 1,
      std::map<long, const wxString>(),
      choices_bool) {;};

  /// Constructor for a ITEM_RADIOBOX, or a ITEM_CHECKLISTBOX_BIT item. 
  /// This checklistbox (not mutually exclusive choices)
  /// can be used to get/set individual bits in a long.
  /// A radiobox (mutually exclusive choices)
  /// should be used when a long value can have a short
  /// set of possible individual values.
  wxExItem(const wxString& label,
    /// the map with values and text
    const std::map<long, const wxString> & choices,
    /// indicates whether to use a radiobox or a checklistbox.
    bool use_radiobox = true,
    const wxString& page = wxEmptyString,
    /// major dimension for the radiobox
    int majorDimension = 0,
    long style = wxRA_SPECIFY_COLS,
    int cols = -1)
    : wxExItem(use_radiobox ? ITEM_RADIOBOX: ITEM_CHECKLISTBOX_BIT, style, page, label, wxEmptyString,
      wxEmptyString, false, false, wxID_ANY, cols, majorDimension, 
      0, 1, 1, 
      choices,
      std::set<wxString>()) {;};

  /// Constructor for a ITEM_USER item.
  wxExItem(const wxString& label,
    /// the window (use default constructor for it)
    wxWindow* window,
    /// callback for window creation (required, useless without one)
    wxExUserWindowCreate create,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    bool add_label = true,
    int cols = -1)
    : wxExItem(ITEM_USER, 0, page, label, wxEmptyString,
      wxEmptyString, is_required, add_label, wxID_ANY, cols, 1, 
      0, 1, 1, 
      std::map<long, const wxString>(),
      std::set<wxString>(),
      window, create) {;};

  /// Constuctor for the other types (as ITEM_BUTTON, ITEM_DIRPICKERCTRL,
  /// ITEM_FILEPICKERCTRL, ITEM_INT).
  wxExItem(
    const wxString& label,
    wxExItemType type,
    /// initial value for the control, if appropriate
    const wxString& value = wxEmptyString,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    /// the id as used by the window, see wxExFrame::OnCommandItemDialog, 
    int id = wxID_ANY,
    bool add_label = true,
    /// the style, this default value is translated to correct default
    /// for corresponging window (such as wxFLP_DEFAULT_STYLE for ITEM_FILEPICKERCTRL).
    long style = 0,
    int cols = -1)
    : wxExItem(type, style, page, label, value, wxEmptyString, is_required, 
        type == ITEM_BUTTON ||
        type == ITEM_CHECKBOX ||
        type == ITEM_COMMAND_LINK_BUTTON ||
        type == ITEM_TOGGLEBUTTON ? false: add_label, id, cols) {;};

  /// Gets the choices.
  const auto & GetChoices() const {return m_Choices;};
  
  /// Gets the number of columns for the current page.
  int GetColumns() const {return m_PageCols;};

  /// Gets is required.
  bool GetIsRequired() const {return m_IsRequired;};

  /// Gets the label.
  const wxString& GetLabel() const {return m_Label;};

  /// Gets the page.
  const wxString& GetPage() const {return m_Page;};

  /// Gets the type.
  wxExItemType GetType() const {return m_Type;};
  
  /// Gets actual value, or IsNull if type
  /// has no (or not yet) associated window, or conversion is not implemented.
  const wxAny GetValue() const;

  /// Gets the window (first call Layout, to create it, otherwise it is NULL).
  wxWindow* GetWindow() const {return m_Window;};
  
  /// Is this item allowed to be expanded on a row.
  bool IsRowGrowable() const {return m_IsRowGrowable;};

  /// Layouts this item (creates the window) on the specified sizer.
  /// It returns the flex grid sizer that was used for creating the item sizer.
  /// Or it returns NULL if no flex grid sizer was used.
  virtual wxFlexGridSizer* Layout(
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
  /// Default whether the item row is growable is determined
  /// by the kind of item. You can ovverride this using SetRowGrowable.
  void SetRowGrowable(bool value) {m_IsRowGrowable = value;};
  
  /// Sets the validator to be used by the item, if appropriate.
  /// Default a normal wxDefaultValidator is used, except for ITEM_INT,
  /// and ITEM_FLOAT.
  void SetValidator(wxValidator* validator) {m_Validator = validator;};
  
  /// Sets actual value for the associated window.
  /// Returns false if window is NULL, or value was not set.
  bool SetValue(const wxAny& value) const;
protected:
  /// Delegate constructor.
  wxExItem(
    /// the item type
    wxExItemType type, 
    /// the style for the control
    long style,
    /// If you specify a page, then all items are placed on that page 
    /// in a book ctrl on the item dialog.
    /// You can specify the number of columns for the page after :
    /// in the page name.
    const wxString& page = wxEmptyString, 
    /// the label to appear in front of the item
    const wxString& label = wxEmptyString, 
    /// intitial value if appropriate
    const wxString& value = wxEmptyString,
    /// some extra info
    const wxString& info = wxEmptyString,
    /// support for the underlying control
    bool is_required = false, 
    /// If you specify add label, then the label is added as a label in front of
    /// the item, otherwise the label is not added
    bool add_label = false,
    /// the window id
    int id = wxID_ANY, 
    /// If you use the default for cols, then the number of cols used
    /// is determined by the dialog, otherwise this number is used.
    int cols = -1, 
    /// major dimention for radio boxes
    int major_dimension = 1,
    /// min value if appropriate
    double min = 0, 
    /// max value if appropriate
    double max = 1, 
    /// increment value if appropriate
    double inc = 1,
    /// choices
    const std::map<long, const wxString> & choices = std::map<long, const wxString>(),
    /// boolean choices
    const std::set<wxString> & choices_bool = std::set<wxString>(),
    /// window, normally created by Layout, but may be supplied here
    wxWindow* window = NULL, 
    /// the process callback for window creation
    wxExUserWindowCreate create = NULL);
private:
  wxFlexGridSizer* AddBrowseButton(wxSizer* sizer) const;
  void AddStaticText(wxSizer* sizer) const;
  void CreateWindow(wxWindow* parent, bool readonly);

  bool m_AddLabel;
  bool m_IsRequired;
  bool m_IsRowGrowable;

  int m_Cols;
  int m_Id;
  int m_MajorDimension;
  int m_PageCols;
  
  double m_Min;
  double m_Max;
  double m_Inc;

  wxString m_Label;
  wxString m_Info;
  wxString m_Page;
  wxString m_Value; // initial value

  long m_Style;

  wxExUserWindowCreate m_UserWindowCreate;
  
  wxExItemType m_Type;
  wxSizerFlags m_SizerFlags;
  wxValidator* m_Validator;
  wxWindow* m_Window;
  
  std::map<long, const wxString> m_Choices;
  std::set<wxString> m_ChoicesBool;
};
#endif // wxUSE_GUI
