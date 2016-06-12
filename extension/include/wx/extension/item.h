////////////////////////////////////////////////////////////////////////////////
// Name:      item.h
// Purpose:   Declaration of wxExItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <utility>
#include <vector>
#include <wx/any.h>
#include <wx/imaglist.h>
#include <wx/sizer.h> // for wxSizer, and wxSizerFlags
#include <wx/string.h>
#include <wx/validate.h>

class wxFlexGridSizer;
class wxWindow;
template <class T> class wxExItemTemplateDialog;

#if wxUSE_GUI
/*! \file */
/// The item types supported.
enum wxExItemType
{
  ITEM_BUTTON,             ///< a wxButton item
  ITEM_CHECKBOX,           ///< a wxCheckBox item
  ITEM_CHECKLISTBOX_BIT,   ///< a wxCheckListBox item to set individual bits in a long
  ITEM_CHECKLISTBOX_BOOL,  ///< a wxCheckListBox item using boolean choices
  ITEM_COLOURPICKERWIDGET, ///< a wxColourPickerWidget item
  ITEM_COMBOBOX,           ///< a wxComboBox item
  ITEM_COMBOBOX_DIR,       ///< a wxComboBox item with a browse button
  ITEM_COMMANDLINKBUTTON,  ///< a wxCommandLinkButton button
  ITEM_DIRPICKERCTRL,      ///< a wxDirPickerCtrl item
  ITEM_EMPTY,              ///< an empty item
  ITEM_FILEPICKERCTRL,     ///< a wxFilePickerCtrl item
  ITEM_FONTPICKERCTRL,     ///< a wxFontPickerCtrl item
  ITEM_HYPERLINKCTRL,      ///< a wxHyperlinkCtrl item
  ITEM_LISTVIEW,           ///< a wxExListView item
  ITEM_NOTEBOOK,           ///< a wxNotebook item
  ITEM_NOTEBOOK_AUI,       ///< a wxAuiNotebook item
  ITEM_NOTEBOOK_CHOICE,    ///< a wxChoicebook item
  ITEM_NOTEBOOK_EX,        ///< a wxExNotebook  item
  ITEM_NOTEBOOK_LIST,      ///< a wxListbook item
  ITEM_NOTEBOOK_SIMPLE,    ///< a wxSimpleNotebook item
  ITEM_NOTEBOOK_TOOL,      ///< a wxToolbook item
  ITEM_NOTEBOOK_TREE,      ///< a wxTreebook item
  ITEM_RADIOBOX,           ///< a wxRadioBox item
  ITEM_SLIDER,             ///< a wxSlider item
  ITEM_SPACER,             ///< a spacer item
  ITEM_SPINCTRL,           ///< a wxSpinCtrl item
  ITEM_SPINCTRLDOUBLE,     ///< a wxSpinCtrlDouble item
  ITEM_STATICLINE,         ///< a wxStaticLine item
  ITEM_STATICTEXT,         ///< a wxStaticText item
  ITEM_STC,                ///< a wxExSTC ctrl item  
  ITEM_TEXTCTRL,           ///< a wxTextCtrl item
  ITEM_TEXTCTRL_FLOAT,     ///< a wxTextCtrl item that only accepts a float (double)
  ITEM_TEXTCTRL_INT,       ///< a wxTextCtrl item that only accepts an integer (long)
  ITEM_TOGGLEBUTTON,       ///< a wxToggleButton item
  ITEM_USER,               ///< provide your own window
};

/// Type label type supported.
enum wxExLabelType
{
  LABEL_NONE,              ///< no label
  LABEL_LEFT,              ///< label left from window
  LABEL_ABOVE,             ///< label above window
};

/// Callback for user window creation.
using wxExUserWindowCreate = void (*)(wxWindow* user, wxWindow* parent, bool readonly);

/// Callback for load or save data for user window.
using wxExUserWindowToConfig = bool (*)(wxWindow* user, bool save);

/// Container class for using with wxExItemDialog.
class WXDLLIMPEXP_BASE wxExItem
{
public:
  typedef std::vector<std::pair<wxString, std::vector<wxExItem>>> ItemsNotebook;
  
  /// Default constructor for a ITEM_EMPTY item.
  wxExItem() : wxExItem(ITEM_EMPTY, 0, wxEmptyString) {;};

  /// Constructor for a ITEM_SPACER item.
  /// The size is the size for the spacer used.
  wxExItem(int size) : wxExItem(ITEM_SPACER, size) {;};

  /// Constuctor for a ITEM_STATICLINE item.
  /// The orientation is wxHORIZONTAL or wxVERTICAL.
  wxExItem(wxOrientation orientation) : wxExItem(ITEM_STATICLINE, orientation) {;};
    
  /// Constructor several items.
  wxExItem(
    /// label for the window as on the dialog,
    /// might also contain the note after a tab for a command link button
    /// if the window supports it you can use a markup label
    const wxString& label,
    /// initial value, also used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& value = wxEmptyString,
    /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
    long style = 0,
    /// type of this item:
    /// - ITEM_HYPERLINKCTRL
    /// - ITEM_STATICTEXT
    /// - ITEM_STC
    /// - ITEM_TEXTCTRL
    wxExItemType type = ITEM_TEXTCTRL,
    /// is this item required 
    bool is_required = false,
    /// will the label be displayed as a static text
    /// ignored for a static text
    wxExLabelType label_type = LABEL_LEFT)
    : wxExItem(type, style, label, value, is_required, 
      (type != ITEM_STATICTEXT && 
       type != ITEM_HYPERLINKCTRL ? label_type: LABEL_NONE), wxID_ANY) {;};

  /// Constructor for spin items.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// min value
    int min, 
    /// max value
    int max,
    /// default value
    const wxAny& value = wxAny(),
    /// type of item: 
    /// - ITEM_SPINCTRL 
    /// - ITEM_SLIDER
    wxExItemType type = ITEM_SPINCTRL,
    /// style for a ITEM_SLIDER item
    long style = wxSL_HORIZONTAL)
    : wxExItem(type, style, label, value,
      false, LABEL_LEFT, wxID_ANY, 1, min, max) {;};

  /// Constructor for a ITEM_SPINCTRLDOUBLE item.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// min value
    double min, 
    /// max value
    double max,
    /// default value
    const wxAny& value = wxAny(),
    /// inc value
    double inc = 1)
    : wxExItem(ITEM_SPINCTRLDOUBLE, 0, label, value,
      false, LABEL_LEFT, wxID_ANY, 1, min, max, inc) {;};

  /// Constructor for a ITEM_CHECKLISTBOX_BOOL item. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices,
    /// style of this item
    long style = 0)
    : wxExItem(ITEM_CHECKLISTBOX_BOOL, style, "checklistbox_bool", choices,
      false, LABEL_NONE, wxID_ANY, 1, 0, 1, 1) {;};

  /// Constuctor for a ITEM_NOTEBOOK item, being a vector
  /// of a pair of pages with a vector of items.
  /// e.g.:
  /// \code
  /// wxExItem("notebook", ItemsNotebook {
  ///   {"page1", 
  ///     {wxExItem("string1"),
  ///      wxExItem("string2"),
  ///      wxExItem("string3")}},
  ///   {"page2", 
  ///     {wxExItem("spin1", 5, 0, 10),
  ///      wxExItem("spin2", 5, 0, 10),
  ///      wxExItem("spin3", 5, 0, 10)}}}, ITEM_NOTEBOOK)
  /// \endcode
  wxExItem(
    /// label for this item
    const wxString& label,
    /// notebook items
    const ItemsNotebook & v,
    /// type of this item (kind of notebook):
    /// - ITEM_NOTEBOOK
    /// - ITEM_NOTEBOOK_AUI
    /// - ITEM_NOTEBOOK_CHOICE
    /// - ITEM_NOTEBOOK_EX
    /// - ITEM_NOTEBOOK_LIST
    /// - ITEM_NOTEBOOK_SIMPLE
    /// - ITEM_NOTEBOOK_TOOL
    /// - ITEM_NOTEBOOK_TREE
    wxExItemType type,
    /// style of this item
    long style = 0,
    /// number of rows
    int rows = 0,
    /// number of cols
    int cols = 1,
    /// type of label
    wxExLabelType label_type = LABEL_NONE,
    /// image list to be used (required for a tool book)
    wxImageList* imageList = nullptr)
  : wxExItem(type, style, label, v, false, 
    label_type, wxID_ANY, cols, 0, 1, 1, nullptr, nullptr, nullptr, imageList) {;};
  
  /// Constructor for a ITEM_RADIOBOX, or a ITEM_CHECKLISTBOX_BIT item. 
  /// This checklistbox (not mutually exclusive choices)
  /// can be used to get/set individual bits in a long.
  /// A radiobox (mutually exclusive choices)
  /// should be used when a long value can have a short
  /// set of possible individual values.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// the map with values and text
    const std::map<long, const wxString> & choices,
    /// indicates whether to use a radiobox or a checklistbox.
    bool use_radiobox = true,
    /// major dimension for the radiobox
    int majorDimension = 1,
    /// style of this item
    long style = wxRA_SPECIFY_COLS)
    : wxExItem(use_radiobox ? ITEM_RADIOBOX: ITEM_CHECKLISTBOX_BIT, style, label, choices,
      false, LABEL_NONE, wxID_ANY, majorDimension, 0, 1, 1) {;};

  /// Constructor for a ITEM_USER item.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// the window (use default constructor for it)
    wxWindow* window,
    /// callback for window creation (required, useless without one)
    wxExUserWindowCreate create,
    /// callback for load and save to config
    /// default it has no relation to the config
    wxExUserWindowToConfig config = nullptr,
    /// is this item required
    bool is_required = false,
    /// type of label
    wxExLabelType label_type = LABEL_LEFT)
    : wxExItem(ITEM_USER, 0, label, wxEmptyString,
      is_required, label_type, wxID_ANY, 1, 0, 1, 1, window, create, config) {;};

  /// Constuctor several items.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// type of item:
    /// - ITEM_BUTTON
    /// - ITEM_COMBOBOX
    /// - ITEM_DIRPICKERCTRL
    /// - ITEM_FILEPICKERCTRL
    /// - ITEM_LISTVIEW
    /// - ITEM_TEXTCTRL_INT
    wxExItemType type,
    /// initial value for the control, if appropriate
    const wxAny& value = wxAny(),
    /// is this item required
    bool is_required = false,
    /// the id as used by the window, see wxExFrame::OnCommandItemDialog, 
    int id = wxID_ANY,
    /// type of label
    wxExLabelType label_type = LABEL_LEFT,
    /// the style, this default value is translated to correct default
    /// for corresponding window (such as wxFLP_DEFAULT_STYLE for ITEM_FILEPICKERCTRL).
    long style = 0)
    : wxExItem(type, style, label, value, is_required, 
        type == ITEM_BUTTON ||
        type == ITEM_CHECKBOX ||
        type == ITEM_COMMANDLINKBUTTON ||
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

  /// Returns true if this item is a notebook.
  bool IsNotebook() const {return m_Type >= ITEM_NOTEBOOK && m_Type <= ITEM_NOTEBOOK_TREE;};
  
  /// Is this item allowed to be expanded on a row.
  bool IsRowGrowable() const {return m_IsRowGrowable;};

  /// Layouts this item (creates the window) on the specified sizer.
  /// It returns the flex grid sizer that was used for creating the item sizer.
  /// Or it returns nullptr if no flex grid sizer was used.
  wxFlexGridSizer* Layout(
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
  
  /// Sets dialog to parent, to allow subitems to be added
  /// to the template dialog.
  void SetDialog(wxExItemTemplateDialog<wxExItem>* dlg);
    
  /// Sets image list.
  void SetImageList(wxImageList* il) {m_ImageList = il;};
  
  /// Sets this item to be growable.
  /// Default whether the item row is growable is determined
  /// by the kind of item. You can ovverride this using SetRowGrowable.
  void SetRowGrowable(bool value) {m_IsRowGrowable = value;};
  
  /// Sets the validator to be used by the item, if appropriate.
  /// Default a normal wxDefaultValidator is used, except for ITEM_TEXTCTRL_INT,
  /// and ITEM_TEXTCTRL_FLOAT.
  void SetValidator(wxValidator* validator) {m_Validator = validator;};
  
  /// Sets actual value for the associated window.
  /// Returns false if window is nullptr, or value was not set.
  bool SetValue(const wxAny& value) const;
  
  /// Loads or saves this item to the config.
  /// Returns true if the config was accessed, as not all
  /// config items associate with the config.
  bool ToConfig(bool save) const;
  
  /// Use config for getting and retrieving values.
  /// Default the config is used.
  static void UseConfig(bool use) {m_UseConfig = use;};
protected:
  /// Delegate constructor.
  wxExItem(
    /// the item type
    wxExItemType type, 
    /// the style for the control
    long style,
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
    wxExUserWindowCreate create = nullptr, 
    /// the process callback for window config
    wxExUserWindowToConfig config = nullptr,
    /// the imagelist
    wxImageList* imageList = nullptr);
private:
  wxFlexGridSizer* Add(wxSizer* sizer, wxFlexGridSizer* current) const;
  wxFlexGridSizer* AddBrowseButton(wxSizer* sizer) const;
  void AddItems(std::pair<wxString, std::vector<wxExItem>> & items, bool readonly);
  wxFlexGridSizer* AddStaticText(wxSizer* sizer) const;
  bool CreateWindow(wxWindow* parent, bool readonly);

  bool m_IsRequired, m_IsRowGrowable = false;
  int m_Id, m_MajorDimension, m_MaxItems = 25;
  long m_Style;
  
  wxExItemType m_Type;
  wxExLabelType m_LabelType;
  wxAny m_Initial, m_Min, m_Max, m_Inc;
  wxString m_Label, m_Page;
  wxSizerFlags m_SizerFlags;
  wxValidator* m_Validator = nullptr;
  wxWindow* m_Window;
  wxImageList* m_ImageList;
  wxExItemTemplateDialog<wxExItem>* m_Dialog = nullptr;
  wxExUserWindowCreate m_UserWindowCreate;
  wxExUserWindowToConfig m_UserWindowToConfig;
  
  static bool m_UseConfig;
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
