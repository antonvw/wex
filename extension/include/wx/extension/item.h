////////////////////////////////////////////////////////////////////////////////
// Name:      item.h
// Purpose:   Declaration of wxExItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <wx/imaglist.h>
#include <wx/sizer.h> // for wxSizer, and wxSizerFlags
#include <wx/slider.h>
#include <wx/string.h>
#include <wx/extension/control-data.h>
#include <wx/extension/listview-data.h>

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
  ITEM_COMBOBOX_DIR,       ///< a wxComboBox item with a browse button for a directory
  ITEM_COMBOBOX_FILE,      ///< a wxComboBox item with a browse button for a file
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

/// Label types supported.
enum wxExLabelType
{
  LABEL_NONE,              ///< no label
  LABEL_LEFT,              ///< label left from window
  LABEL_ABOVE,             ///< label above window
};

/// Container class for using with wxExItemDialog.
/// The next items can be set using specified control data:
/// - Id: as used by the window, see wxExFrame::OnCommandItemDialog, 
/// - Required: ensures that the control must have a value otherwise OK, APPLY is not enabled,
/// - Style: the default value is translated to correct default
/// 
/// For corresponding window (such as wxFLP_DEFAULT_STYLE for ITEM_FILEPICKERCTRL)
/// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD).
class WXDLLIMPEXP_BASE wxExItem
{
public:
  /// Choices for radioboxes.
  typedef std::map<long, const char*> Choices;
    
  /// This is vector of a pair of pages with a vector of items.
  typedef std::vector<std::pair<wxString, std::vector<wxExItem>>> 
    ItemsNotebook;
  
  /// A function that you can provide to e.g. specify what 
  /// to do when clicking on a button item.
  typedef std::function<void(wxWindow* user, const std::any& value, bool save)> 
    UserApply;
  
  /// A function that you can provide to specify what needs to
  /// be done for creating a user item.
  typedef std::function<void(wxWindow* user, wxWindow* parent, bool readonly)> 
    UserWindowCreate;
  
  /// A function that you can provide to specify what needs to
  /// be done of loading or saving a user item to
  /// the config.
  typedef std::function<bool(wxWindow* user, bool save)> 
    UserWindowToConfig;

  /// Default constructor for a ITEM_EMPTY item.
  wxExItem() : wxExItem(ITEM_EMPTY, wxEmptyString) {;};

  /// Constructor for a ITEM_SPACER item.
  /// The size is the size for the spacer used.
  wxExItem(int size) : wxExItem(ITEM_SPACER) {
    m_Data.Window(wxExWindowData().Style(size));};

  /// Constuctor for a ITEM_STATICLINE item.
  /// The orientation is wxHORIZONTAL or wxVERTICAL.
  wxExItem(wxOrientation orientation) : wxExItem(ITEM_STATICLINE) {
    m_Data.Window(wxExWindowData().Style(orientation));};
    
  /// Constructor for several items.
  wxExItem(
    /// label for the window as on the dialog,
    /// might also contain the note after a tab for a command link button
    /// if the window supports it you can use a markup label
    const wxString& label,
    /// initial value, also used as default for a hyperlink ctrl, or as lexer for STC
    const wxString& value = wxEmptyString,
    /// type of this item:
    /// - ITEM_HYPERLINKCTRL
    /// - ITEM_STATICTEXT
    /// - ITEM_STC
    /// - ITEM_TEXTCTRL
    wxExItemType type = ITEM_TEXTCTRL,
    /// control data
    const wxExControlData& data = wxExControlData(),
    /// will the label be displayed as a static text
    /// ignored for a static text
    wxExLabelType label_type = LABEL_LEFT,
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(type, label, value, 
      (type != ITEM_STATICTEXT && 
       type != ITEM_HYPERLINKCTRL ? label_type: LABEL_NONE))
      {m_Apply = apply;
       m_Data = data;};

  /// Constructor for spin items.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// min value
    int min, 
    /// max value
    int max,
    /// default value
    const std::any& value = std::any(),
    /// type of item: 
    /// - ITEM_SPINCTRL 
    /// - ITEM_SLIDER
    wxExItemType type = ITEM_SPINCTRL,
    /// control data
    const wxExControlData& data = wxExControlData().Window(wxExWindowData().Style(wxSL_HORIZONTAL)),
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(type, label, value, LABEL_LEFT, 1, min, max)
      {m_Apply = apply;
       m_Data = data;};

  /// Constructor for a ITEM_SPINCTRLDOUBLE item.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// min value
    double min, 
    /// max value
    double max,
    /// default value
    const std::any& value = std::any(),
    /// inc value
    double inc = 1,
    /// control data
    const wxExControlData& data = wxExControlData().Window(wxExWindowData().Style(wxSL_HORIZONTAL)),
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(ITEM_SPINCTRLDOUBLE, label, value, LABEL_LEFT, 1, min, max, inc)
      {m_Apply = apply;
       m_Data = data;};

  /// Constructor for a ITEM_CHECKLISTBOX_BOOL item. 
  /// This checklistbox can be used to get/set several boolean values.
  wxExItem(
    /// the set with names of boolean items
    const std::set<wxString> & choices,
    /// control data
    const wxExControlData& data = wxExControlData(),
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(ITEM_CHECKLISTBOX_BOOL, "checklistbox_bool", choices, LABEL_NONE, 1, 0, 1, 1) 
      {m_Apply = apply;
       m_Data = data;};

  /// Constuctor for a ITEM_NOTEBOOK item, being a vector
  /// of a pair of pages with a vector of items.
  /// e.g.:
  /// \code
  /// wxExItem("notebook", {
  ///   {"page1", 
  ///     {{"string1"},
  ///      {"string2"},
  ///      {"string3"}}},
  ///   {"page2", 
  ///     {{"spin1", 5, 0, 10},
  ///      {"spin2", 5, 0, 10},
  ///      {"spin3", 5, 0, 10}}}})
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
#ifdef __WXMSW__
    wxExItemType type = ITEM_NOTEBOOK_LIST,
#else
    wxExItemType type = ITEM_NOTEBOOK,
#endif
    /// number of rows
    int rows = 0,
    /// number of cols
    int cols = 1,
    /// control data
    const wxExControlData& data = wxExControlData(),
    /// type of label
    wxExLabelType label_type = LABEL_NONE,
    /// image list to be used (required for a tool book)
    wxImageList* imageList = nullptr)
    : wxExItem(type, label, v, label_type, cols, 0, 1, 1, nullptr, nullptr, nullptr, imageList) {
        m_Data= data;};
  
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
    const Choices & choices,
    /// indicates whether to use a radiobox or a checklistbox.
    bool use_radiobox = true,
    /// major dimension for the radiobox
    int majorDimension = 1,
    /// control data
    const wxExControlData& data = wxExControlData().Window(wxExWindowData().Style(wxRA_SPECIFY_COLS)),
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(use_radiobox ? ITEM_RADIOBOX: ITEM_CHECKLISTBOX_BIT, label, choices,
      LABEL_NONE, majorDimension, 0, 1, 1) {
        m_Apply = apply;
        m_Data = data;};

  /// Constructor for a ITEM_USER item.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// the window (use default constructor for it)
    wxWindow* window,
    /// callback for window creation (required, useless without one)
    UserWindowCreate create,
    /// callback for load and save to config
    /// if nullptr it has no relation to the config
    UserWindowToConfig config = nullptr,
    /// type of label
    wxExLabelType label_type = LABEL_LEFT,
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(ITEM_USER, label, wxEmptyString, label_type, 1, 0, 1, 1, window, create, config) {
        m_Apply = apply;};

  /// Constuctor a ITEM_LISTVIEW item.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// listview data
    const wxExListViewData& data,
    /// initial value
    const std::any& value = std::any(),
    /// type of label
    wxExLabelType label_type = LABEL_NONE,
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(ITEM_LISTVIEW, label, value, label_type) {
        m_Apply = apply;
        m_ListViewData = data;};

  /// Constuctor several items.
  wxExItem(
    /// label for this item
    const wxString& label,
    /// type of item:
    /// - ITEM_BUTTON
    /// - ITEM_CHECKBOX
    /// - ITEM_COLOURPICKERWIDGET
    /// - ITEM_COMBOBOX
    /// - ITEM_COMBOBOX_DIR
    /// - ITEM_COMBOBOX_FILE
    /// - ITEM_COMMANDLINKBUTTON
    /// - ITEM_DIRPICKERCTRL
    /// - ITEM_FILEPICKERCTRL
    /// - ITEM_FONTPICKERCTRL
    /// - ITEM_TEXTCTRL_FLOAT
    /// - ITEM_TEXTCTRL_INT
    /// - ITEM_TOGGLEBUTTON
    wxExItemType type,
    /// initial value for the control, if appropriate
    const std::any& value = std::any(),
    /// control data
    const wxExControlData& data = wxExControlData(),
    /// type of label
    wxExLabelType label_type = LABEL_LEFT,
    /// callback to apply
    UserApply apply = nullptr)
    : wxExItem(type, label, value, 
        type == ITEM_BUTTON ||
        type == ITEM_CHECKBOX ||
        type == ITEM_COMMANDLINKBUTTON ||
        type == ITEM_TOGGLEBUTTON ? LABEL_NONE: label_type) {
        m_Apply = apply;
        m_Data = data;};
  
  /// If apply callback has been provided calls apply.
  /// Otherwise return false.
  bool Apply(bool save = true) const {
    if (m_Apply != nullptr) 
    {
      (m_Apply)(m_Window, GetValue(), save);
      return true;
    }
    return false;};

  /// Returns the number of columns for the current page.
  auto GetColumns() const {return m_MajorDimension;};

  /// Returns control data.
  const auto& GetData() const {return m_Data;};

  /// Returns the initial value.
  const auto& GetInitial() const {return m_Initial;};
  
  /// Returns the label.
  const auto& GetLabel() const {return m_Label;};

  /// Returns the page.
  const auto& GetPage() const {return m_Page;};

  /// Returns the type.
  auto GetType() const {return m_Type;};
  
  /// Returns actual value, or empty object if this item
  /// has no (or not yet) associated window, or conversion is not implemented.
  const std::any GetValue() const;

  /// Returns the window (first call Layout, to create it, otherwise it is nullptr).
  auto* GetWindow() const {return m_Window;};

  /// Is this item allowed to be expanded on a row.
  auto IsRowGrowable() const {return m_IsRowGrowable;};

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

  /// Logs info about this item.
  std::stringstream Log() const;

  /// Sets dialog to parent, to allow subitems to be added
  /// to the template dialog.
  void SetDialog(wxExItemTemplateDialog<wxExItem>* dlg);
    
  /// Sets image list.
  void SetImageList(wxImageList* il) {m_ImageList = il;};
  
  /// Sets this item to be growable.
  /// Default whether the item row is growable is determined
  /// by the kind of item. You can override this using SetRowGrowable.
  void SetRowGrowable(bool value) {m_IsRowGrowable = value;};
  
  /// Sets actual value for the associated window.
  /// Returns false if window is nullptr, or value was not set.
  bool SetValue(const std::any& value) const;
  
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
    /// the label to appear in front of the item
    const wxString& label = wxEmptyString, 
    /// intitial value if appropriate
    const std::any& value = wxString(),
    /// If you specify add label, then the label is added as a label in front of
    /// the item, otherwise the label is not added
    wxExLabelType label_type = LABEL_NONE,
    /// major dimention for radio boxes
    int major_dimension = 1,
    /// min value if appropriate
    const std::any& min = 0, 
    /// max value if appropriate
    const std::any& max = 1, 
    /// increment value if appropriate
    const std::any& inc = 1,
    /// window, normally created by Layout, but may be supplied here
    wxWindow* window = nullptr, 
    /// the process callback for window creation
    UserWindowCreate create = nullptr, 
    /// the process callback for window config
    UserWindowToConfig config = nullptr,
    /// the imagelist
    wxImageList* imageList = nullptr);
private:
  wxFlexGridSizer* Add(wxSizer* sizer, wxFlexGridSizer* current) const;
  wxFlexGridSizer* AddBrowseButton(wxSizer* sizer);
  void AddItems(std::pair<wxString, std::vector<wxExItem>> & items, bool readonly);
  wxFlexGridSizer* AddStaticText(wxSizer* sizer) const;
  bool CreateWindow(wxWindow* parent, bool readonly);
  std::stringstream Log(const std::string& name, const std::any& any) const;

  bool m_IsRowGrowable = false;
  int m_MajorDimension, m_MaxItems = 25;
  
  wxExItemType m_Type;
  wxExLabelType m_LabelType;
  std::any m_Initial, m_Min, m_Max, m_Inc;
  wxString m_Label, m_Page;
  wxSizerFlags m_SizerFlags;
  wxWindow* m_Window;
  wxImageList* m_ImageList;
  wxExItemTemplateDialog<wxExItem>* m_Dialog = nullptr;
  wxExControlData m_Data;
  wxExListViewData m_ListViewData;

  UserApply m_Apply;
  UserWindowCreate m_UserWindowCreate;
  UserWindowToConfig m_UserWindowToConfig;
  
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
    const std::vector<std::tuple<wxString, wxExItemType, std::any>> & items);
  
  /// Destructor, stops recording.
 ~wxExConfigDefaults();
  
  /// Access to config.
  auto* Get() {return m_Config;};
private:
  wxConfigBase* m_Config;
};
#endif // wxUSE_GUI
