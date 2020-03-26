////////////////////////////////////////////////////////////////////////////////
// Name:      item.h
// Purpose:   Declaration of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include <wex/control-data.h>
#include <wex/listview-data.h>
#include <wex/log.h>
#include <wx/imaglist.h>
#include <wx/sizer.h> // for wxSizer, and wxSizerFlags
#include <wx/slider.h>

class wxFlexGridSizer;
class wxWindow;

namespace wex
{
  template <class T> class item_template_dialog;

  /*! \file */
  /// Container class for using with item_dialog.
  /// The next items can be set using specified control_data:
  /// - id: as used by the window, see frame::on_command_item_dialog,
  /// - is_required: ensures that the control must have a value otherwise OK,
  ///   APPLY is not enabled,
  /// - style: the default value is translated to correct default
  ///
  /// For corresponding window (such as wxFLP_DEFAULT_STYLE for FILEPICKERCTRL)
  /// the style for the control used (e.g. wxTE_MULTILINE or wxTE_PASSWORD).
  /// If the window supports it you can use a markup label.
  class item
  {
  public:
    /// The item types supported.
    enum type_t
    {
      /// wxButton item
      BUTTON,

      /// wxCheckBox item
      CHECKBOX,

      /// wxCheckListBox item to set individual bits in a long
      CHECKLISTBOX_BIT,

      /// wxCheckListBox item using boolean choices
      CHECKLISTBOX_BOOL,

      /// wxColourPickerWidget item
      COLOURPICKERWIDGET,

      /// wxComboBox item
      COMBOBOX,

      /// wxComboBox item with a browse button for a directory
      COMBOBOX_DIR,

      /// wxComboBox item with a browse button for a file
      COMBOBOX_FILE,

      /// wxCommandLinkButton button
      COMMANDLINKBUTTON,

      /// wxDirPickerCtrl item
      DIRPICKERCTRL,

      /// empty item
      EMPTY,

      /// wxFilePickerCtrl item
      FILEPICKERCTRL,

      /// wxFontPickerCtrl item
      FONTPICKERCTRL,

      /// wex::grid item
      GRID,

      /// wxHyperlinkCtrl item
      HYPERLINKCTRL,

      /// wex::listview item
      LISTVIEW,

      /// wxNotebook item
      NOTEBOOK,

      /// wxAuiNotebook item
      NOTEBOOK_AUI,

      /// wxChoicebook item
      NOTEBOOK_CHOICE,

      /// wxListbook item
      NOTEBOOK_LIST,

      /// wxSimpleNotebook item
      NOTEBOOK_SIMPLE,

      /// wxToolbook item
      NOTEBOOK_TOOL,

      /// wxTreebook item
      NOTEBOOK_TREE,

      /// wex::notebook item
      NOTEBOOK_WEX,

      /// wxRadioBox item
      RADIOBOX,

      /// wxSlider item
      SLIDER,

      /// spacer item
      SPACER,

      /// wxSpinCtrl item
      SPINCTRL,

      /// wxSpinCtrlDouble item
      SPINCTRLDOUBLE,

      /// wxStaticBox item
      STATICBOX,

      /// wxStaticLine item
      STATICLINE,

      /// wxStaticText item
      STATICTEXT,

      /// wex::stc item
      STC,

      /// wxTextCtrl item
      TEXTCTRL,

      /// wxTextCtrl item that only accepts a float (double)
      TEXTCTRL_FLOAT,

      /// wxTextCtrl item that only accepts an integer (long)
      TEXTCTRL_INT,

      /// wxToggleButton item
      TOGGLEBUTTON,

      /// provide your own window
      USER,
    };

    /// Label types supported.
    enum label_t
    {
      LABEL_NONE,  ///< no label
      LABEL_LEFT,  ///< label left from window
      LABEL_ABOVE, ///< label above window
    };

    /// Choices for radioboxes.
    typedef std::map<
      /// value
      long,
      /// name, default the value is not set,
      /// but can be set by adding , '1' to the name
      const std::string>
      choices_t;

    /// Choices for listboxes with toggle options.
    typedef std::set<std::string> choices_bool_t;

    // A group is a pair of text with a vector of items.
    typedef std::pair<std::string, std::vector<item>> group_t;

    /// A notebook is a vector of groups.
    typedef std::vector<group_t> notebook_t;

    /// A function that you can provide to e.g. specify what
    /// to do when clicking on a button item.
    typedef std::function<
      void(wxWindow* user, const std::any& value, bool save)>
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

    /// Sets dialog to parent, to allow subitems to be added
    /// to the template dialog.
    static void set_dialog(item_template_dialog<item>* dlg);

    /// Use config for getting and retrieving values.
    /// Default the config is used.
    /// The label is used as entry in the config.
    static void use_config(bool use) { m_use_config = use; };

    /// Default constructor for an EMPTY item.
    item()
      : item(EMPTY, std::string())
    {
      ;
    };

    /// Constructor for a SPACER item.
    /// The size is the size for the spacer used.
    item(int size)
      : item(SPACER)
    {
      m_data.window(window_data().style(size));
    };

    /// Constructor for a STATICLINE item.
    /// The orientation is wxHORIZONTAL or wxVERTICAL.
    item(wxOrientation orientation)
      : item(STATICLINE)
    {
      m_data.window(window_data().style(orientation));
    };

    /// Constructor for several items.
    item(
      /// label for the window as on the dialog,
      /// might also contain the note after a tab for a command link button
      /// you can use a parent child config item by using a
      /// dot in the label, the prefix is not shown on the window
      const std::string& label,
      /// initial value, also used as default for a hyperlink ctrl,
      /// or as lexer for STC
      const std::string& value = std::string(),
      /// type of this item:
      /// - GRID
      /// - HYPERLINKCTRL
      /// - STATICTEXT
      /// - STC
      /// - TEXTCTRL
      /// if the label contains a colon, it is a STATICTEXT,
      /// otherwise default TEXTCTRL
      /// if type is STATICTEXT then markup is allowed in the label text
      type_t type = TEXTCTRL,
      /// control data
      const control_data& data = control_data(),
      /// will the label be displayed as a static text
      /// ignored for a static text
      label_t label_t = LABEL_LEFT,
      /// callback to apply
      user_apply_t apply = nullptr);

    /// Constructor for a SPINCTRL or a SLIDER item.
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
      const control_data& data =
        control_data().window(window_data().style(wxSP_ARROW_KEYS)),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(type, label, value, LABEL_LEFT, 1, min, max)
    {
      m_apply = apply;
      m_data  = data;
    };

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
      const control_data& data =
        control_data().window(window_data().style(wxSP_ARROW_KEYS)),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(SPINCTRLDOUBLE, label, value, LABEL_LEFT, 1, min, max, inc)
    {
      m_apply = apply;
      m_data  = data;
    };

    /// Constructor for a CHECKLISTBOX_BOOL item.
    /// This checklistbox can be used to get/set several boolean values.
    item(
      /// the set with names of boolean items
      /// the default value is false, but can be changed by adding a
      /// ',1' postfix to the name
      const choices_bool_t& choices,
      /// control data
      const control_data& data = control_data(),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(
          CHECKLISTBOX_BOOL,
          "checklistbox_bool",
          choices,
          LABEL_NONE,
          1,
          0,
          1,
          1)
    {
      m_apply = apply;
      m_data  = data;
    };

    /// Constructor for a NOTEBOOK item, being a vector
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
      const notebook_t& v,
    /// type of this item (kind of notebook):
    /// - NOTEBOOK
    /// - NOTEBOOK_AUI
    /// - NOTEBOOK_CHOICE
    /// - NOTEBOOK_LIST
    /// - NOTEBOOK_SIMPLE
    /// - NOTEBOOK_TOOL
    /// - NOTEBOOK_TREE
    /// - NOTEBOOK_WEX
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
      : item(
          type,
          label,
          v,
          label_t,
          cols,
          0,
          1,
          1,
          nullptr,
          nullptr,
          nullptr,
          imageList)
    {
      m_data = data;
    };

    /// Constructor for a STATICBOX item.
    item(
      /// group items
      const group_t& v,
      /// control data
      const control_data& data = control_data())
      : item(STATICBOX, v.first, v)
    {
      m_data = data;
    };

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
      const choices_t& choices,
      /// indicates whether to use a radiobox or a checklistbox.
      bool use_radiobox = true,
      /// major dimension for the radiobox
      int major_dimension = 1,
      /// control data
      const control_data& data =
        control_data().window(window_data().style(wxRA_SPECIFY_COLS)),
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(
          use_radiobox ? RADIOBOX : CHECKLISTBOX_BIT,
          label,
          choices,
          LABEL_NONE,
          major_dimension,
          0,
          1,
          1)
    {
      m_apply = apply;
      m_data  = data;
    };

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
      : item(
          USER,
          label,
          std::string(),
          label_t,
          1,
          0,
          1,
          1,
          window,
          create,
          config)
    {
      m_apply = apply;
    };

    /// Constructor a LISTVIEW item.
    item(
      /// label for this item
      const std::string& label,
      /// listview data
      const listview_data& data,
      /// initial value
      /// expects std::list< std::string>
      const std::any& value = std::any(),
      /// type of label
      label_t label_t = LABEL_NONE,
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(LISTVIEW, label, value, label_t)
    {
      m_apply         = apply;
      m_listview_data = data;
    };

    /// Constructor several items.
    item(
      /// label for this item
      /// if type is BUTTON then markup is allowed in the label text
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
      /// initial value for the control, if appropriate:
      /// - CHECKBOX expects bool
      /// - COMBOXBOX expects std::list< std::string>
      /// - COLOURPICKERWIDGET expects a wxColour
      /// - TEXTCTRL_FLOAT expects std::string with float contents
      /// - TEXTCTRL_INT expects std::string with int contents
      const std::any& value = std::any(),
      /// control data
      const control_data& data = control_data(),
      /// type of label
      label_t label_t = LABEL_LEFT,
      /// callback to apply
      user_apply_t apply = nullptr)
      : item(
          type,
          label,
          value,
          type == BUTTON || type == CHECKBOX || type == COMMANDLINKBUTTON ||
              type == TOGGLEBUTTON ?
            LABEL_NONE :
            label_t)
    {
      m_apply = apply;
      m_data  = data;
    };

    /// If apply callback has been provided calls apply.
    /// Otherwise return false.
    bool apply(bool save = true) const
    {
      if (m_apply != nullptr)
      {
        (m_apply)(m_window, get_value(), save);
        return true;
      }
      return false;
    };

    /// Returns the number of columns for the current page.
    auto columns() const { return m_major_dimension; };

    /// Returns control data.
    const auto& data() const { return m_data; };

    /// Returns true if this item is empty.
    bool empty() const { return m_type == EMPTY; };

    /// Returns actual value, or empty object if this item
    /// has no (or not yet) associated window, or conversion is not implemented.
    const std::any get_value() const;

    /// Returns the initial value.
    const auto& initial() const { return m_initial; };

    /// Returns true if this item is a notebook.
    bool is_notebook() const;

    /// Is this item allowed to be expanded on a row.
    auto is_row_growable() const { return m_is_row_growable; };

    /// Returns the label.
    const auto& label() const { return m_label; };

    /// layouts this item (creates the window) on the specified sizer.
    /// It returns the flex grid sizer that was used for creating the item
    /// sizer. Or it returns nullptr if no flex grid sizer was used.
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
    const auto& page() const { return m_page; };

    /// Sets image list.
    void set_imagelist(wxImageList* il) { m_image_list = il; };

    /// Sets this item to be growable.
    /// Default whether the item row is growable is determined
    /// by the kind of item. You can override this using SetRowGrowable.
    void set_row_growable(bool value) { m_is_row_growable = value; };

    /// Sets actual value for the associated window.
    /// Returns false if window is nullptr, or value was not set.
    bool set_value(const std::any& value) const;

    /// Loads or saves this item to the config.
    /// Returns true if the config was accessed, as not all
    /// config items associate with the config.
    bool to_config(bool save) const;

    /// Returns the type.
    auto type() const { return m_type; };

    /// Returns the window (first call layout, to create it,
    /// otherwise it is nullptr).
    auto* window() const { return m_window; };

  protected:
    /// Delegate constructor.
    item(
      /// the item type
      type_t type,
      /// the label to appear in front of the item
      const std::string& label = std::string(),
      /// initial value if appropriate
      const std::any& value = std::string(),
      /// If you specify add label, then the label is added as a label in front
      /// of the item, otherwise the label is not added
      label_t label_t = LABEL_NONE,
      /// major dimension for radio boxes
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
    wxFlexGridSizer* add(wxSizer* sizer, wxFlexGridSizer* current) const;
    wxFlexGridSizer* add_browse_button(wxSizer* sizer) const;
    void             add_items(group_t& page, bool readonly);
    void             add_items(
                  wxWindow*          parent,
                  wxFlexGridSizer*   sizer,
                  std::vector<item>& v,
                  bool               readonly);
    wxFlexGridSizer* add_static_text(wxSizer* sizer) const;
    bool             create_window(wxWindow* parent, bool readonly);

    bool m_is_row_growable = false;

    int m_major_dimension, m_max_items{25};

    type_t  m_type;
    label_t m_label_type;

    std::any m_initial, m_min, m_max, m_inc;

    std::string m_label, m_label_window, m_page;

    control_data  m_data;
    listview_data m_listview_data;

    user_apply_t            m_apply;
    user_window_create_t    m_user_window_create_t;
    user_window_to_config_t m_user_window_to_config_t;

    wxImageList* m_image_list;
    wxSizerFlags m_sizer_flags;
    wxWindow*    m_window;

    static inline item_template_dialog<item>* m_dialog     = nullptr;
    static inline bool                        m_use_config = true;
  };
}; // namespace wex
