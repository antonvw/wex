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

  /// a dirpicker ctrl item
  CONFIG_DIRPICKERCTRL,

  /// a filepicker ctrl item
  CONFIG_FILEPICKERCTRL,

  /// a fontpicker ctrl item
  CONFIG_FONTPICKERCTRL,

  /// a hyperlink ctrl item
  CONFIG_HYPERLINKCTRL,

  /// a textctrl item that only accepts an integer (long)
  CONFIG_INT,

  /// a radiobox item
  CONFIG_RADIOBOX,

  /// a slider item
  CONFIG_SLIDER,

  /// a spinctrl item
  CONFIG_SPINCTRL,

  /// a spinctrl double item
  CONFIG_SPINCTRL_DOUBLE,

  /// a static line item (default horizontal)
  CONFIG_STATICLINE,

  /// a static text item
  CONFIG_STATICTEXT,

  /// a textctrl item
  CONFIG_STRING,

  /// a toggle button item
  CONFIG_TOGGLEBUTTON,
  
  /// provide your own control
  CONFIG_USER,

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
class WXDLLIMPEXP_BASE wxExConfigItem
{
public:
  /// Constuctor for most types.
  wxExConfigItem(const wxString& name,
    wxExConfigType type,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    ///< the id as used by the control, 
    ///< when using for a combobox dir, use id < wxID_LOWEST
    ///< accessible using GetControl()->GetId()
    int id = wxID_ANY,
    ///< used by CONFIG_COMBOBOX
    int max_items = 25,
    ///< will the name be displayed as a static text
    bool add_name = true,
    int cols = -1,
    ///< extra style, only used for static line
    long style = 0);
    
  /// Constructor for a user control.
  /// Default it has no relation to the config,
  /// if you want to, you have to implement UserControlToConfig
  /// in your derived class.
  wxExConfigItem(
    ///< name for the control as on the dialog and in the config
    const wxString& name,
    ///< the control (use default constructor for it)
    wxControl* control,
    ///< the page
    const wxString& page = wxEmptyString,
    ///< is this control required
    bool is_required = false,
    ///< will the name be displayed as a static text
    bool add_name = true,
    ///< number of cols for this control
    int cols = -1);

  /// Constructor for a string, a hyperlink ctrl or a static text.
  /// The extra style argument is the style for the control used
  /// (e.g. wxTE_MULTILINE or wxTE_PASSWORD).
  wxExConfigItem(const wxString& name,
    ///< used as default for a hyperlink ctrl
    const wxString& value = wxEmptyString,
    const wxString& page = wxEmptyString,
    long style = 0,
    wxExConfigType type = CONFIG_STRING,
    bool is_required = false,
    ///< ignored for a static text
    bool add_name = true,
    int cols = -1);

  /// Constructor for a spin ctrl, a spin ctrl double or a slider.
  wxExConfigItem(const wxString& name,
    double min, double max,
    const wxString& page = wxEmptyString,
    wxExConfigType type = CONFIG_SPINCTRL,
    long style = wxSL_HORIZONTAL,
    double inc = 1,
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
  wxExConfigType GetType() const {return m_Type;};

  /// Creates the control,
  /// lays out this item on the specified sizer, and fills it
  /// with config value (calls ToConfig).
  /// It returns the flex grid sizer that was used for creating the item sizer.
  /// Or it returns NULL if no flex grid sizer was used.
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
  /// Returns true if the config was accessed, as not all
  /// config items associate with the config.
  bool ToConfig(bool save) const;
protected:
  /// Creates the user control item, using default Create method.
  /// Override if you need an other Create method.
  virtual void UserControlCreate(wxWindow* parent, bool readonly) const {
    m_Control->Create(parent, m_Id);};
  /// Allows you to load or save config data for your control.
  virtual bool UserControlToConfig(bool save) const {return false;};
private:
  wxFlexGridSizer* AddBrowseButton(wxSizer* sizer) const;
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
  
  double m_Min;
  double m_Max;
  double m_Inc;

  wxString m_Name;
  wxString m_Page;
  wxString m_Default; // used by hyperlink as default web address

  long m_Style;

  std::map<long, const wxString> m_Choices;
  std::set<wxString> m_ChoicesBool;

  wxExConfigType m_Type;
  wxControl* m_Control;
  wxSizerFlags m_ControlFlags;
};
#endif // wxUSE_GUI
#endif
