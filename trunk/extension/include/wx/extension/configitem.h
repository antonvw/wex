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

#if wxUSE_GUI
/*! \file */
/// The config item types supported.
enum
{
  /// A checklistbox (not mutually exclusive choices).
  /// Should be used to get/set individual bits in a long.
  CONFIG_CHECKLISTBOX,

  /// A checklistbox without a name (not mutually exclusive choices).
  /// Should be used to get/set several boolean values in one checklistbox.
  CONFIG_CHECKLISTBOX_NONAME,

  /// A radiobox (mutually exclusive choices).
  /// Should be used when a long value can have a short set of possible individual values.
  CONFIG_RADIOBOX,

  /// A normal, single, checkbox (use ReadBool to retrieve value).
  CONFIG_CHECKBOX,       

  CONFIG_COLOUR,         ///< a colour button
  CONFIG_COMBOBOX,       ///< a combobox
  CONFIG_COMBOBOXDIR,    ///< a combobox with a browse button
  CONFIG_DIRPICKERCTRL,  ///< a dirpicker ctrl
  CONFIG_FILEPICKERCTRL, ///< a filepicker ctrl
  CONFIG_FONTPICKERCTRL, ///< a fontpicker ctrl
  CONFIG_INT,            ///< a textctrl that only accepts an integer (a long integer)
  CONFIG_SPINCTRL,       ///< a spinctrl
  CONFIG_SPINCTRL_DOUBLE, ///< a spinctrl double
  CONFIG_STRING,         ///< a textctrl

  CONFIG_SPACER,         ///< a spacer only, no config item
};

/// Container class for using with wxExConfigDialog.
/// If you specify a page, then all items are placed on that page in a notebook.
class wxExConfigItem
{
  friend class wxExConfigDialog;
public:
  /// Default contructor (for a spacer item).
  wxExConfigItem();

  /// Constructor for a spin ctrl.
  wxExConfigItem(const wxString& name,
    int min, int max,
    const wxString& page = wxEmptyString);

  /// Constructor for a spin ctrl double.
  wxExConfigItem(const wxString& name,
    double min, double max, double inc = 1,
    const wxString& page = wxEmptyString);

  /// Constructor for a string.
  /// The extra style argument is the style for the wxTextCtrl used.
  /// (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
  wxExConfigItem(const wxString& name,
    const wxString& page = wxEmptyString,
    long style = 0,
    bool is_required = false);

  /// Constructor for a radiobox or a checklistbox. Just specify
  /// the map with values and text.
  wxExConfigItem(const wxString& name,
    const std::map<long, const wxString> & choices,
    bool use_radiobox = true,
    const wxString& page = wxEmptyString);

  /// Constructor for a checklistbox without a name. Just specify
  /// the map with values and text.
  wxExConfigItem(const std::set<wxString> & choices,
    const wxString& page = wxEmptyString);

  /// Constuctor for other types.
  wxExConfigItem(const wxString& name,
    int type,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    int max_items = 25); // used by CONFIG_COMBOBOX
private:
  // cannot be const, otherwise
  // error C2582: 'operator =' function is unavailable in 'wxExConfigItem'
  bool m_IsRequired;
  int m_Min;
  int m_Max;
  int m_MaxItems;
  double m_MinDouble;
  double m_MaxDouble;
  double m_Inc;
  wxString m_Name;
  wxString m_Page;
  long m_Style;
  int m_Type;
  wxControl* m_Control;
  std::map<long, const wxString> m_Choices;
  std::set<wxString> m_ChoicesBool;
};
#endif // wxUSE_GUI
#endif
