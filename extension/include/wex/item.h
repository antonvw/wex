////////////////////////////////////////////////////////////////////////////////
// Name:      item.h
// Purpose:   Declaration of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
#include <wex/control-data.h>
#include <wex/listview-data.h>

class wxFlexGridSizer;
class wxWindow;
  
namespace wex
{
  template <class T> class item_template_dialog;

  /*! \file */
  /// Container class for using with item_dialog.
  /// The next items can be set using specified control data:
  /// - id: as used by the window, see frame::on_command_item_dialog, 
  /// - is_required: ensures that the control must have a value otherwise OK, APPLY is not enabled,
  /// - style: the default value is translated to correct default
  /// 
  /// For corresponding window (such as wxFLP_DEFAULT_STYLE for FILEPICKERCTRL)
  /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD).
  class item
  {
  public:
    /// The item types supported.
    enum type_t
    {
      BUTTON,             ///< a wxButton item
      CHECKBOX,           ///< a wxCheckBox item
      CHECKLISTBOX_BIT,   ///< a wxCheckListBox item to set individual bits in a long
      CHECKLISTBOX_BOOL,  ///< a wxCheckListBox item using boolean choices
      COLOURPICKERWIDGET, ///< a wxColourPickerWidget item
      COMBOBOX,           ///< a wxComboBox item
      COMBOBOX_DIR,       ///< a wxComboBox item with a browse button for a directory
      COMBOBOX_FILE,      ///< a wxComboBox item with a browse button for a file
      COMMANDLINKBUTTON,  ///< a wxCommandLinkButton button
      DIRPICKERCTRL,      ///< a wxDirPickerCtrl item
      EMPTY,              ///< an empty item
      FILEPICKERCTRL,     ///< a wxFilePickerCtrl item
      FONTPICKERCTRL,     ///< a wxFontPickerCtrl item
      HYPERLINKCTRL,      ///< a wxHyperlinkCtrl item
      LISTVIEW,           ///< a wex::listview item
      NOTEBOOK,           ///< a wxNotebook item
      NOTEBOOK_AUI,       ///< a wxAuiNotebook item
      NOTEBOOK_CHOICE,    ///< a wxChoicebook item
      NOTEBOOK_EX,        ///< a wex::notebook  item
      NOTEBOOK_LIST,      ///< a wxListbook item
      NOTEBOOK_SIMPLE,    ///< a wxSimpleNotebook item
      NOTEBOOK_TOOL,      ///< a wxToolbook item
      NOTEBOOK_TREE,      ///< a wxTreebook item
      RADIOBOX,           ///< a wxRadioBox item
      SLIDER,             ///< a wxSlider item
      SPACER,             ///< a spacer item
      SPINCTRL,           ///< a wxSpinCtrl item
      SPINCTRLDOUBLE,     ///< a wxSpinCtrlDouble item
      STATICLINE,         ///< a wxStaticLine item
      STATICTEXT,         ///< a wxStaticText item
      STC,                ///< a wex::stc ctrl item  
      TEXTCTRL,           ///< a wxTextCtrl item
      TEXTCTRL_FLOAT,     ///< a wxTextCtrl item that only accepts a float (double)
      TEXTCTRL_INT,       ///< a wxTextCtrl item that only accepts an integer (long)
      TOGGLEBUTTON,       ///< a wxToggleButton item
      USER,               ///< provide your own window
    };

    /// Label types supported.
    enum label_t
    {
      LABEL_NONE,              ///< no label
      LABEL_LEFT,              ///< label left from window
      LABEL_ABOVE,             ///< label above window
    };

    /// Choices for radioboxes.
    typedef std::map<long, const std::string> choices_t;
      
    /// This is vector of a pair of pages with a vector of items.
    typedef std::vector<std::pair<std::string, std::vector<item>>> 
      items_notebook_t;
    
    /// A function that you can provide to e.g. specify what 
    /// to do when clicking on a button item.
    typedef std::function<void(wxWindow* user, const std::any& value, bool save)> 
      user_apply_t;
    
    /// A function that you can provide to specify what needs to
    /// be done for creating a user item.
    typedef std::function<void(wxWindow* user, wxWindow* parent, bool readonly)> 
      user_window_create_t;
    
    /// A function that you can provide to specify what needs to
    /// be done of loading or saving a user item to
    /// the config.
    typedef std::function<bool(wxWindow* user, bool save)> 
      user_window_to_config_t;

    /// Default constructor for a EMPTY item.
    item() : item(EMPTY, std::string()) {;};

    /// Constructor for a SPACER item.
    /// The size is the size for the spacer used.
    item(int size) : item(SPACER) {
      m_Data.window(window_data().style(size));};

    /// Constuctor for a STATICLINE item.
    /// The orientation is wxHORIZONTAL or wxVERTICAL.
    item(wxOrientation orientation) : item(STATICLINE) {
      m_Data.window(window_data().style(orientation));};
      
    /// Constructor for several items.
    item(
      /// label for the window as on the dialog,
      /// might also contain the note after a tab for a command link button
      /// if the window supports it you can use a markup label
      const std::string& label,
      /// initial value, also used as default for a hyperlink ctrl, or as lexer for STC
      const std::string& value = std::string(),
      /// type of this item:
      /// - HYPERLINKCTRL
      /// - STATICTEXT
      /// - STC
      /// - TEXTCTRL
      type_t type = TEXTCTRL,
      /// control data
      const control_data& data = control_data(),
      /// will the label be displayed as a static text
      /// ignored for a static text
      label_t label_t = LABEL_LEFT,
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(type, label, value, 
        (type != STATICTEXT && 
         type != HYPERLINKCTRL ? label_t: LABEL_NONE))
        {m_apply = apply;
         m_Data = data;};

    /// Constructor for spin items.
    item(
      /// label for this item
      const std::string& label,
      /// min value
      int min, 
      /// max value
      int max,
      /// default value
      const std::any& value = std::any(),
      /// type of item: 
      /// - SPINCTRL 
      /// - SLIDER
      type_t type = SPINCTRL,
      /// control data
      const control_data& data = control_data().window(window_data().style(wxSL_HORIZONTAL)),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(type, label, value, LABEL_LEFT, 1, min, max)
        {m_apply = apply;
         m_Data = data;};

    /// Constructor for a SPINCTRLDOUBLE item.
    item(
      /// label for this item
      const std::string& label,
      /// min value
      double min, 
      /// max value
      double max,
      /// default value
      const std::any& value = std::any(),
      /// inc value
      double inc = 1,
      /// control data
      const control_data& data = control_data().window(window_data().style(wxSL_HORIZONTAL)),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(SPINCTRLDOUBLE, label, value, LABEL_LEFT, 1, min, max, inc)
        {m_apply = apply;
         m_Data = data;};

    /// Constructor for a CHECKLISTBOX_BOOL item. 
    /// This checklistbox can be used to get/set several boolean values.
    item(
      /// the set with names of boolean items
      const std::set<std::string> & choices,
      /// control data
      const control_data& data = control_data(),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(CHECKLISTBOX_BOOL, "checklistbox_bool", choices, LABEL_NONE, 1, 0, 1, 1) 
        {m_apply = apply;
         m_Data = data;};

    /// Constuctor for a NOTEBOOK item, being a vector
    /// of a pair of pages with a vector of items.
    /// e.g.:
    /// \code
    /// wex::item("notebook", {
    ///   {"page1", 
    ///     {{"string1"},
    ///      {"string2"},
    ///      {"string3"}}},
    ///   {"page2", 
    ///     {{"spin1", 5, 0, 10},
    ///      {"spin2", 5, 0, 10},
    ///      {"spin3", 5, 0, 10}}}})
    /// \endcode
    item(
      /// label for this item
      const std::string& label,
      /// notebook items
      const items_notebook_t & v,
      /// type of this item (kind of notebook):
      /// - NOTEBOOK
      /// - NOTEBOOK_AUI
      /// - NOTEBOOK_CHOICE
      /// - NOTEBOOK_EX
      /// - NOTEBOOK_LIST
      /// - NOTEBOOK_SIMPLE
      /// - NOTEBOOK_TOOL
      /// - NOTEBOOK_TREE
#ifdef __WXMSW__
      type_t type = NOTEBOOK_LIST,
#else
      type_t type = NOTEBOOK,
#endif
      /// number of rows
      int rows = 0,
      /// number of cols
      int cols = 1,
      /// control data
      const control_data& data = control_data(),
      /// type of label
      label_t label_t = LABEL_NONE,
      /// image list to be used (required for a tool book)
      wxImageList* imageList = nullptr)
      : item(type, label, v, label_t, cols, 0, 1, 1, nullptr, nullptr, nullptr, imageList) {
          m_Data= data;};
    
    /// Constructor for a RADIOBOX, or a CHECKLISTBOX_BIT item. 
    /// This checklistbox (not mutually exclusive choices)
    /// can be used to get/set individual bits in a long.
    /// A radiobox (mutually exclusive choices)
    /// should be used when a long value can have a short
    /// set of possible individual values.
    item(
      /// label for this item
      const std::string& label,
      /// the map with values and text
      const choices_t & choices,
      /// indicates whether to use a radiobox or a checklistbox.
      bool use_radiobox = true,
      /// major dimension for the radiobox
      int majorDimension = 1,
      /// control data
      const control_data& data = control_data().window(window_data().style(wxRA_SPECIFY_COLS)),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(use_radiobox ? RADIOBOX: CHECKLISTBOX_BIT, label, choices,
        LABEL_NONE, majorDimension, 0, 1, 1) {
          m_apply = apply;
          m_Data = data;};

    /// Constructor for a USER item.
    item(
      /// label for this item
      const std::string& label,
      /// the window (use default constructor for it)
      wxWindow* window,
      /// callback for window creation (required, useless without one)
      user_window_create_t create,
      /// callback for load and save to config
      /// if nullptr it has no relation to the config
      user_window_to_config_t config = nullptr,
      /// type of label
      label_t label_t = LABEL_LEFT,
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(USER, label, std::string(), label_t, 1, 0, 1, 1, window, create, config) {
          m_apply = apply;};

    /// Constuctor a LISTVIEW item.
    item(
      /// label for this item
      const std::string& label,
      /// listview data
      const listview_data& data,
      /// initial value
      const std::any& value = std::any(),
      /// type of label
      label_t label_t = LABEL_NONE,
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(LISTVIEW, label, value, label_t) {
          m_apply = apply;
          m_ListViewData = data;};

    /// Constuctor several items.
    item(
      /// label for this item
      const std::string& label,
      /// type of item:
      /// - BUTTON
      /// - CHECKBOX
      /// - COLOURPICKERWIDGET
      /// - COMBOBOX
      /// - COMBOBOX_DIR
      /// - COMBOBOX_FILE
      /// - COMMANDLINKBUTTON
      /// - DIRPICKERCTRL
      /// - FILEPICKERCTRL
      /// - FONTPICKERCTRL
      /// - TEXTCTRL_FLOAT
      /// - TEXTCTRL_INT
      /// - TOGGLEBUTTON
      type_t type,
      /// initial value for the control, if appropriate
      const std::any& value = std::any(),
      /// control data
      const control_data& data = control_data(),
      /// type of label
      label_t label_t = LABEL_LEFT,
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(type, label, value, 
          type == BUTTON ||
          type == CHECKBOX ||
          type == COMMANDLINKBUTTON ||
          type == TOGGLEBUTTON ? LABEL_NONE: label_t) {
          m_apply = apply;
          m_Data = data;};
    
    /// If apply callback has been provided calls apply.
    /// Otherwise return false.
    bool apply(bool save = true) const {
      if (m_apply != nullptr) 
      {
        (m_apply)(m_Window, get_value(), save);
        return true;
      }
      return false;};

    /// Returns the number of columns for the current page.
    auto columns() const {return m_MajorDimension;};

    /// Returns control data.
    const auto& data() const {return m_Data;};

    /// Returns actual value, or empty object if this item
    /// has no (or not yet) associated window, or conversion is not implemented.
    const std::any get_value() const;

    /// Returns the initial value.
    const auto& initial() const {return m_Initial;};
    
    /// Is this item allowed to be expanded on a row.
    auto is_row_growable() const {return m_is_row_growable;};

    /// Returns the label.
    const auto& label() const {return m_Label;};

    /// layouts this item (creates the window) on the specified sizer.
    /// It returns the flex grid sizer that was used for creating the item sizer.
    /// Or it returns nullptr if no flex grid sizer was used.
    wxFlexGridSizer* layout(
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
    std::stringstream log() const;

    /// Returns the page.
    const auto& page() const {return m_Page;};

    /// Sets dialog to parent, to allow subitems to be added
    /// to the template dialog.
    void set_dialog(item_template_dialog<item>* dlg);
      
    /// Sets image list.
    void set_imagelist(wxImageList* il) {m_ImageList = il;};
    
    /// Sets this item to be growable.
    /// Default whether the item row is growable is determined
    /// by the kind of item. You can override this using SetRowGrowable.
    void set_row_growable(bool value) {m_is_row_growable = value;};
    
    /// Sets actual value for the associated window.
    /// Returns false if window is nullptr, or value was not set.
    bool set_value(const std::any& value) const;
    
    /// Loads or saves this item to the config.
    /// Returns true if the config was accessed, as not all
    /// config items associate with the config.
    bool to_config(bool save) const;
    
    /// Returns the type.
    auto type() const {return m_Type;};
    
    /// Use config for getting and retrieving values.
    /// Default the config is used.
    static void use_config(bool use) {m_use_config = use;};
    
    /// Returns the window (first call layout, to create it, otherwise it is nullptr).
    auto* window() const {return m_Window;};
  protected:
    /// Delegate constructor.
    item(
      /// the item type
      type_t type, 
      /// the label to appear in front of the item
      const std::string& label = std::string(), 
      /// intitial value if appropriate
      const std::any& value = std::string(),
      /// If you specify add label, then the label is added as a label in front of
      /// the item, otherwise the label is not added
      label_t label_t = LABEL_NONE,
      /// major dimention for radio boxes
      int major_dimension = 1,
      /// min value if appropriate
      const std::any& min = 0, 
      /// max value if appropriate
      const std::any& max = 1, 
      /// increment value if appropriate
      const std::any& inc = 1,
      /// window, normally created by layout, but may be supplied here
      wxWindow* window = nullptr, 
      /// the process callback for window creation
      user_window_create_t create = nullptr, 
      /// the process callback for window config
      user_window_to_config_t config = nullptr,
      /// the imagelist
      wxImageList* imageList = nullptr);
  private:
    wxFlexGridSizer* Add(wxSizer* sizer, wxFlexGridSizer* current) const;
    wxFlexGridSizer* AddBrowseButton(wxSizer* sizer);
    void add_items(std::pair<std::string, std::vector<item>> & items, bool readonly);
    wxFlexGridSizer* AddStaticText(wxSizer* sizer) const;
    bool CreateWindow(wxWindow* parent, bool readonly);
    std::stringstream Log(const std::string& name, const std::any& any) const;

    bool m_is_row_growable = false;
    int m_MajorDimension, m_MaxItems = 25;
    
    type_t m_Type;
    label_t m_LabelType;
    std::any m_Initial, m_Min, m_Max, m_Inc;
    std::string m_Label, m_Page;
    wxSizerFlags m_SizerFlags;
    wxWindow* m_Window;
    wxImageList* m_ImageList;
    item_template_dialog<item>* m_Dialog = nullptr;
    control_data m_Data;
    listview_data m_ListViewData;

    user_apply_t m_apply;
    user_window_create_t m_user_window_create_t;
    user_window_to_config_t m_user_window_to_config_t;
    
    static inline bool m_use_config = true;
  };

  /// Support class for keeping defaults in the config.
  class config_defaults
  {
  public:
    /// A default with name, item type, and default value.
    typedef std::tuple<std::string, item::type_t, std::any> Defaults;

    /// Constructor, records default values,
    /// if not yet in the config.
    config_defaults(const std::vector<Defaults> & items);
    
    /// Destructor, stops recording.
   ~config_defaults();
  };
};
