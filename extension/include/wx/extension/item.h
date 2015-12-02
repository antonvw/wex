////////////////////////////////////////////////////////////////////////////////
// Name:      item.h
// Purpose:   Declaration of wxExItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <utility>
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
  ITEM_LISTVIEW,             ///< a wxExListView ctrl item (a list view standard file)
  ITEM_NOTEBOOK,             ///< a traditional notebook item
  ITEM_NOTEBOOK_AUI,         ///< a aui notebook
  ITEM_NOTEBOOK_CHOICE,      ///< a choice book
  ITEM_NOTEBOOK_EX,          ///< a wxExNotebook 
  ITEM_NOTEBOOK_LIST,        ///< a list book
  ITEM_NOTEBOOK_SIMPLE,      ///< a simple notebook
  ITEM_NOTEBOOK_TOOL,        ///< a tool book
  ITEM_NOTEBOOK_TREE,        ///< a tree book
  ITEM_RADIOBOX,             ///< a wxRadioBox item
  ITEM_SLIDER,               ///< a wxSlider item
  ITEM_SPACER,               ///< a spacer item
  ITEM_SPINCTRL,             ///< a wxSpinCtrl item
  ITEM_SPINCTRL_DOUBLE,      ///< a wxSpinCtrlDouble item
  ITEM_STATICLINE,           ///< a wxStaticLine item
  ITEM_STATICTEXT,           ///< a wxStaticText item
  ITEM_STC,                  ///< a wxExSTC ctrl item  
  ITEM_STRING,               ///< a wxTextCtrl item
  ITEM_TOGGLEBUTTON,         ///< a wxToggleButton item
  ITEM_USER,                 ///< provide your own window
};

enum wxExLabelType
{
  LABEL_NONE,                ///< no label
  LABEL_LEFT,                ///< label left from window
  LABEL_ABOVE,               ///< label above window
};

/// Callback for user window creation.
typedef void (*wxExUserWindowCreate)(wxWindow* user, wxWindow* parent, bool readonly);

/// Container class for using with wxExItemDialog.
class WXDLLIMPEXP_BASE wxExItem
{
public:
  typedef std::vector<std::pair<wxString, std::vector<wxExItem>>> ItemsNotebook;
  
  /// Default constructor for a ITEM_EMPTY item.
  wxExItem() : wxExItem(ITEM_EMPTY, 0, wxEmptyString) {;};

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
    /// initial value, also used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& value = wxEmptyString,
    const wxString& page = wxEmptyString,
    /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
    long style = 0,
    wxExItemType type = ITEM_STRING,
    bool is_required = false,
    /// will the label be displayed as a static text
    /// ignored for a static text
    wxExLabelType label_type = LABEL_LEFT)
    : wxExItem(type, style, page, label, value, is_required, 
      (type != ITEM_STATICTEXT && 
       type != ITEM_HYPERLINKCTRL ? label_type: LABEL_NONE), wxID_ANY) {;};

  /// Constructor for a ITEM_SPINCTRL or a ITEM_SLIDER item.
  wxExItem(const wxString& label,
    int value,
    int min, 
    int max,
    const wxString& page = wxEmptyString,
    wxExItemType type = ITEM_SPINCTRL,
    /// style for a ITEM_SLIDER item
    long style = wxSL_HORIZONTAL)
    : wxExItem(type, style, page, label, value,
      false, LABEL_LEFT, wxID_ANY, 1, min, max) {;};

  /// Constructor for a ITEM_SPINCTRL_DOUBLE item.
  wxExItem(const wxString& label,
    double value,
    double min, 
    double max,
    const wxString& page = wxEmptyString,
    double inc = 1)
    : wxExItem(ITEM_SPINCTRL_DOUBLE, 0, page, label, value,
      false, LABEL_LEFT, wxID_ANY, 1, min, max, inc) {;};

  /// Constructor for a ITEM_CHECKLISTBOX_BOOL item. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices,
    const wxString& page = wxEmptyString,
    long style = 0)
    : wxExItem(ITEM_CHECKLISTBOX_BOOL, style, page, "checklistbox_bool", choices,
      false, LABEL_NONE, wxID_ANY, 1, 0, 1, 1) {;};

  /// Constuctor for a ITEM_NOTEBOOK item, being a vector
  /// of a pair of pages with a vector of items.
  /// e.g.:
  /// wxExItem(ItemsNotebook {
  ///   {"page1", 
  ///     {wxExItem("string1"),
  ///      wxExItem("string2"),
  ///      wxExItem("string3")}},
  ///   {"page2", 
  ///     {wxExItem("spin1", 5, 0, 10),
  ///      wxExItem("spin2", 5, 0, 10),
  ///      wxExItem("spin3", 5, 0, 10)}}}
  wxExItem(
    const ItemsNotebook & v,
    wxExItemType type,
    long style,
    int rows,
    int cols)
  : wxExItem(type, style, wxEmptyString, wxEmptyString, v, false, 
    LABEL_NONE, wxID_ANY, cols) {;};
  
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
    int majorDimension = 1,
    long style = wxRA_SPECIFY_COLS)
    : wxExItem(use_radiobox ? ITEM_RADIOBOX: ITEM_CHECKLISTBOX_BIT, style, page, label, choices,
      false, LABEL_NONE, wxID_ANY, majorDimension, 0, 1, 1) {;};

  /// Constructor for a ITEM_USER item.
  wxExItem(const wxString& label,
    /// the window (use default constructor for it)
    wxWindow* window,
    /// callback for window creation (required, useless without one)
    wxExUserWindowCreate create,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    wxExLabelType label_type = LABEL_LEFT)
    : wxExItem(ITEM_USER, 0, page, label, wxEmptyString,
      is_required, label_type, wxID_ANY, 1, 0, 1, 1, window, create) {;};

  /// Constuctor for the other types (as ITEM_BUTTON, 
  /// ITEM_COMBOBOX, ITEM_DIRPICKERCTRL,
  /// ITEM_FILEPICKERCTRL, ITEM_INT, ITEM_LISTVIEW).
  wxExItem(
    const wxString& label,
    wxExItemType type,
    /// initial value for the control, if appropriate
    const wxAny& value = wxAny(),
    const wxString& page = wxEmptyString,
    bool is_required = false,
    /// the id as used by the window, see wxExFrame::OnCommandItemDialog, 
    int id = wxID_ANY,
    wxExLabelType label_type = LABEL_LEFT,
    /// the style, this default value is translated to correct default
    /// for corresponding window (such as wxFLP_DEFAULT_STYLE for ITEM_FILEPICKERCTRL).
    long style = 0)
    : wxExItem(type, style, page, label, value, is_required, 
        type == ITEM_BUTTON ||
        type == ITEM_CHECKBOX ||
        type == ITEM_COMMAND_LINK_BUTTON ||
        type == ITEM_TOGGLEBUTTON ? LABEL_NONE: label_type, id) {;};

  /// Returns the number of columns for the current page.
  int GetColumns() const {return m_MajorDimension;};

  /// Returns the initial value.
  const auto & GetInitial() const {return m_Initial;};
  
  /// Returns is required.
  bool GetIsRequired() const {return m_IsRequired;};

  /// Returns the label.
  const wxString& GetLabel() const {return m_Label;};

  /// Returns the page.
  const wxString& GetPage() const {return m_Page;};

  /// Returns the type.
  auto GetType() const {return m_Type;};
  
  /// Returns actual value, or IsNull if type
  /// has no (or not yet) associated window, or conversion is not implemented.
  const wxAny GetValue() const;

  /// Returns the window (first call Layout, to create it, otherwise it is nullptr).
  wxWindow* GetWindow() const {return m_Window;};
  
  /// Is this item allowed to be expanded on a row.
  bool IsRowGrowable() const {return m_IsRowGrowable;};

  /// Layouts this item (creates the window) on the specified sizer.
  /// It returns the flex grid sizer that was used for creating the item sizer.
  /// Or it returns nullptr if no flex grid sizer was used.
  virtual wxFlexGridSizer* Layout(
    /// the parent
    wxWindow* parent, 
    /// the sizer
    wxSizer* sizer,
    /// specify the item will be readonly, it will not be changeable
    /// if underlying control supports this
    bool readonly = false,
    /// specify the sizer for creating the item, or nullptr,
    /// than a new one is created
    wxFlexGridSizer* fgz = nullptr);
    
  /// Sets this item to be growable.
  /// Default whether the item row is growable is determined
  /// by the kind of item. You can ovverride this using SetRowGrowable.
  void SetRowGrowable(bool value) {m_IsRowGrowable = value;};
  
  /// Sets the validator to be used by the item, if appropriate.
  /// Default a normal wxDefaultValidator is used, except for ITEM_INT,
  /// and ITEM_FLOAT.
  void SetValidator(wxValidator* validator) {m_Validator = validator;};
  
  /// Sets actual value for the associated window.
  /// Returns false if window is nullptr, or value was not set.
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
    const wxAny& value = wxString(),
    /// support for the underlying control
    bool is_required = false, 
    /// If you specify add label, then the label is added as a label in front of
    /// the item, otherwise the label is not added
    wxExLabelType label_type = LABEL_NONE,
    /// the window id
    int id = wxID_ANY, 
    /// major dimention for radio boxes
    int major_dimension = 1,
    /// min value if appropriate
    const wxAny& min = 0, 
    /// max value if appropriate
    const wxAny& max = 1, 
    /// increment value if appropriate
    const wxAny& inc = 1,
    /// window, normally created by Layout, but may be supplied here
    wxWindow* window = nullptr, 
    /// the process callback for window creation
    wxExUserWindowCreate create = nullptr);
private:
  wxFlexGridSizer* Add(wxSizer* sizer, wxFlexGridSizer* current) const;
  wxFlexGridSizer* AddBrowseButton(wxSizer* sizer) const;
  wxFlexGridSizer* AddStaticText(wxSizer* sizer) const;
  bool CreateWindow(wxWindow* parent, bool readonly);

  bool m_IsRequired, m_IsRowGrowable;
  int m_Id, m_MajorDimension;
  long m_Style;
  
  wxExItemType m_Type;
  wxExLabelType m_LabelType;
  wxAny m_Initial, m_Min, m_Max, m_Inc;
  wxString m_Label, m_Page;
  wxSizerFlags m_SizerFlags;
  wxValidator* m_Validator;
  wxWindow* m_Window;
  wxExUserWindowCreate m_UserWindowCreate;
};
#endif // wxUSE_GUI
