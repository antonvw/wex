////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.h
// Purpose:   Declaration of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Created:   2009-11-10
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXCONFIGITEM_H
#define _EXCONFIGITEM_H

#include <map>
#include <set>
#include <wx/control.h>
#include <wx/sizer.h>
#include <wx/slider.h> // for wxSL_HORIZONTAL
#include <wx/string.h>

#if wxUSE_GUI
/*! \file */
/// The config item types supported.
enum
{
  /// Used for automatic testing only.
  CONFIG_ITEM_MIN,

  /// Items that are constructed using the default constructor.
  /// a button
  CONFIG_BUTTON,
  
  /// a checkbox (use ReadBool to retrieve value)
  CONFIG_CHECKBOX,

  /// a colour button
  CONFIG_COLOUR,
  
  /// a combobox
  CONFIG_COMBOBOX,

  /// a combobox with a browse button
  CONFIG_COMBOBOXDIR,

  /// a dirpicker ctrl
  CONFIG_DIRPICKERCTRL,

  /// a filepicker ctrl
  CONFIG_FILEPICKERCTRL,

  /// a fontpicker ctrl
  CONFIG_FONTPICKERCTRL,

  /// a hyperlink ctrl
  CONFIG_HYPERLINKCTRL,

  /// a textctrl that only accepts an integer (long)
  CONFIG_INT,

  /// a static line
  CONFIG_STATICLINE,

  /// a static text
  CONFIG_STATICTEXT,

   /// a textctrl
  CONFIG_STRING,

  // Items that have an explicit constructor.
  /// a checklistbox ctrl
  CONFIG_CHECKLISTBOX,

  /// a checklistbox ctrl
  CONFIG_CHECKLISTBOX_NONAME,

  /// a radiobox ctrl
  CONFIG_RADIOBOX,

  /// a slider
  CONFIG_SLIDER,

  /// a spinctrl
  CONFIG_SPINCTRL,

  /// a spinctrl double
  CONFIG_SPINCTRL_DOUBLE,

  /// Used for automatic testing only.
  CONFIG_ITEM_MAX,
};

/// Container class for using with wxExConfigDialog.
/// If you specify a page, then all items are placed on that page in a notebook.
/// If you specify add name, then the name is added as a label to
/// the item as well, otherwise the name is not added, and only used
/// for loading and saving from config.
/// If you use the default for cols, then the number of cols used
/// is determined by the config dialog, otherwise this number is used.
class wxExConfigItem
{
public:
  /// Default constuctor.
  /// When using for a combobox dir, use id < wxID_LOWEST.
  wxExConfigItem(const wxString& name = wxEmptyString,
    int type = CONFIG_STATICLINE,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    /// the id as used by the control, use GetControl()->GetId()
    /// to retrieve actual value
    int id = wxID_ANY,
    /// used by CONFIG_COMBOBOX
    int max_items = 25,
    /// will the name be displayed as a static text
    bool add_name = true,
    int cols = -1);

  /// Constructor for a string, a hyperlink ctrl or a static text.
  /// The extra style argument is the style for the control used
  /// (e.g. wxTE_MULTILINE or wxTE_PASSWORD).
  wxExConfigItem(const wxString& name,
    const wxString& value = wxEmptyString,
    const wxString& page = wxEmptyString,
    long style = 0,
    int type = CONFIG_STRING,
    bool is_required = false,
    ///< ignored for a static text
    bool add_name = true,
    int cols = -1);

  /// Constructor for a checklistbox without a name. Just specify
  /// the set with names of boolean items.
  /// A checklistbox without a name (not mutually exclusive choices)
  /// should be used to get/set several boolean values in one checklistbox.
  wxExConfigItem(const std::set<wxString> & choices,
    const wxString& page = wxEmptyString,
    int cols = -1);

  /// Constructor for a radiobox or a checklistbox. Just specify
  /// the map with values and text.
  /// A checklistbox (not mutually exclusive choices)
  /// should be used to get/set individual bits in a long.
  /// A radiobox (mutually exclusive choices)
  /// should be used when a long value can have a short
  /// set of possible individual values.
  wxExConfigItem(const wxString& name,
    const std::map<long, const wxString> & choices,
    bool use_radiobox = true,
    const wxString& page = wxEmptyString,
    int majorDimension = 0,
    long style = wxRA_SPECIFY_COLS,
    int cols = -1);

  /// Constructor for a spin ctrl or a slider.
  wxExConfigItem(const wxString& name,
    int min, int max,
    const wxString& page = wxEmptyString,
    bool spin = true,
    long style = wxSL_HORIZONTAL,
    int cols = -1);

  /// Constructor for a spin ctrl double.
  wxExConfigItem(const wxString& name,
    double min, double max, double inc = 1,
    const wxString& page = wxEmptyString,
    int cols = -1);

  /// Gets the columns.
  int GetColumns() const {return m_Cols;};

  /// Gets the control (first call Layout).
  wxControl* GetControl() const {return m_Control;};

  /// Gets is required.
  bool GetIsRequired() const {return m_IsRequired;};

  /// Gets the name.
  const wxString& GetName() const {return m_Name;};

  /// Gets the page.
  const wxString& GetPage() const {return m_Page;};

  /// Gets the type.
  int GetType() const {return m_Type;};

  /// Creates the control,
  /// lays out this item on the specified sizer, and fills it
  /// with config value (calls ToConfig).
  /// It returns the sizer that was used for creating the item sizer.
  wxFlexGridSizer* Layout(
    wxWindow* parent, 
    wxSizer* sizer,
    ///< specify the item will be readonly, it will not be changeable
    ///< if underlying control supports this
    bool readonly = false,
    ///< specify the sizer for creating the item, or NULL,
    ///< than a new one is created
    wxFlexGridSizer* fgz = NULL);

  /// Loads or saves this item to the config.
  void ToConfig(bool save) const;
private:
  void AddBrowseButton(wxSizer* sizer) const;
  void AddStaticTextName(wxSizer* sizer) const;
  /// Creates the control.
  void CreateControl(wxWindow* parent, bool readonly);

  // The members are allowed to be const using
  // MS Visual Studio 2010, not using gcc, so
  // removed again (operator= seems to be used).
  bool m_AddName;
  bool m_IsRequired;

  int m_Cols;
  int m_Id;
  int m_MajorDimension;
  int m_MaxItems;
  int m_Type;

  double m_Min;
  double m_Max;
  double m_Inc;

  wxString m_Name;
  wxString m_Page;
  wxString m_Default;

  long m_Style;

  std::map<long, const wxString> m_Choices;
  std::set<wxString> m_ChoicesBool;

  wxControl* m_Control;
  wxSizerFlags m_ControlFlags;
};
#endif // wxUSE_GUI
#endif
