/******************************************************************************\
* File:          config.h
* Purpose:       Declaration of wxWidgets config extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXCONFIG_H
#define _EXCONFIG_H

#include <wx/regex.h>
#include <wx/fdrepdlg.h>

class exFindReplaceData;

#ifdef EX_PORTABLE
#include <wx/fileconf.h>
class exConfig : public wxFileConfig
#else
#include <wx/config.h>

/// Offers a general configuration.
/// Keys are read the first time accessed from the config.
/// Next time they are retrieved from the maps, so access is fast.
class exConfig : public wxConfig
#endif
{
public:
#ifdef EX_PORTABLE
  /// Default constructor.
  exConfig(const wxString& filename = wxEmptyString);
#else
  /// Default constructor.
  exConfig();
#endif
  /// Destructor, writes all keys.
 ~exConfig();

  /// Gets the key as a long. If the key is not present,
  /// it is added to the map of long values.
  long Get(
    const wxString& key, long default_value) {
    std::map<wxString, long>::const_iterator it = m_LongValues.find(key);

    if (it != m_LongValues.end())
    {
      return it->second;
    }
    else
    {
      m_LongValues.insert(std::make_pair(key, Read(key, default_value)));
      return m_LongValues[key];
    }
  }

  /// Gets the key as a string. If the key is not present,
  /// it is added to the map of string values.
  /// This also works for comboboxes,
  /// as long as the values are separated by default row delimiter,
  /// as then it returns value before this delimiter.
  const wxString Get(
    const wxString& key,
    const wxString& default_value = wxEmptyString,
    const wxChar field_separator = ',') {
    std::map<wxString, wxString>::const_iterator it = m_StringValues.find(key);

    if (it != m_StringValues.end())
    {
      const wxString value = it->second;
      return value.BeforeFirst(field_separator);
    }  
    else
    {
      m_StringValues.insert(std::make_pair(key, Read(key, default_value)));
      return m_StringValues[key];
    }
  }

  /// Gets the key as a bool. If the key is not present,
  /// it is added to the map of bool values.
  bool GetBool(
    const wxString& key, bool default_value = true) {
    std::map<wxString, bool>::const_iterator it = m_BoolValues.find(key);

    if (it != m_BoolValues.end())
    {
      return it->second;
    }
    else
    {
      m_BoolValues.insert(std::make_pair(key, ReadBool(key, default_value)));
      return m_BoolValues[key];
    }
  }

  /// Gets the find replace data.
  exFindReplaceData* GetFindReplaceData() const {
    return m_FindReplaceData;};

  /// Sets key as a long.
  void Set(const wxString& key, long value) {
    m_LongValues[key] = value;};

  /// Sets key as a string.
  void Set(const wxString& key, const wxString& value) {
    m_StringValues[key] = value;};

  /// Sets key as a bool.
  void SetBool(const wxString& key, bool value) {
    m_BoolValues[key] = value;};

  /// Toggles boolean key value.
  void Toggle(const wxString& key) {
    m_BoolValues[key] = !m_BoolValues[key];}
private:
  exFindReplaceData* m_FindReplaceData;
  std::map<wxString, bool> m_BoolValues;
  std::map<wxString, long> m_LongValues;
  std::map<wxString, wxString> m_StringValues;
};

#if wxUSE_GUI
/*! \file */
/// The config item types supported.
enum
{
  CONFIG_CHECKBOX,       ///< a checkbox (use GetBool to retrieve value)
  CONFIG_CHECKLISTBOX,   ///< a checklistbox (not mutually exclusive choices)
  CONFIG_COLOUR,         ///< a colour button
  CONFIG_COMBOBOX,       ///< a combobox
  CONFIG_COMBOBOXDIR,    ///< a combobox with a browse button
  CONFIG_DIRPICKERCTRL,  ///< a dirpicker ctrl
  CONFIG_FILEPICKERCTRL, ///< a filepicker ctrl
  CONFIG_FONTPICKERCTRL, ///< a fontpicker ctrl
  CONFIG_INT,            ///< a textctrl that only accepts an integer (a long integer)
  CONFIG_RADIOBOX,       ///< a radiobox (mutually exclusive choices)
  CONFIG_SPINCTRL,       ///< a spinctrl
  CONFIG_STRING,         ///< a textctrl
};

/// Container class for using with exConfigDialog.
/// If you specify a page, then all items are placed on that page in a notebook.
class exConfigItem
{
  friend class exConfigDialog;
public:
  /// Constructor for a spin ctrl.
  exConfigItem(const wxString& name,
    int min,
    int max,
    const wxString& page = wxEmptyString)
  : m_IsRequired(false)
  , m_Min(min)
  , m_Max(max)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_SPINCTRL) {;};

  /// Constructor for a string.
  /// The extra style argument is the style for the wxTextCtrl used.
  /// (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
  exConfigItem(const wxString& name,
    const wxString& page = wxEmptyString,
    long style = 0,
    bool is_required = false)
  : m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(style)
  , m_Type(CONFIG_STRING) {;};

  /// Constructor for a radiobox or a checklistbox. Just specify
  /// the map with values and text.
  exConfigItem(const wxString& name,
    const std::map<int, const wxString> & choices,
    bool use_radiobox = true,
    const wxString& page = wxEmptyString)
  : m_IsRequired(false)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(use_radiobox ? CONFIG_RADIOBOX: CONFIG_CHECKLISTBOX)
  , m_Choices(choices) {;};

  /// Constuctor for other types.
  exConfigItem(const wxString& name,
    int type,
    const wxString& page = wxEmptyString,
    bool is_required = false,
    int max_items = 25) // used by CONFIG_COMBOBOX
  : m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(max_items)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(type) {;};
private:
  // cannot be const, otherwise
  // error C2582: 'operator =' function is unavailable in 'exConfigItem'
  bool m_IsRequired;
  int m_Min;
  int m_Max;
  int m_MaxItems;
  wxString m_Name;
  wxString m_Page;
  long m_Style;
  int m_Type;
  wxControl* m_Control;
  std::map<int, const wxString> m_Choices;
};
#endif // wxUSE_GUI

#if wxUSE_GUI
/// Offers a dialog to set several items in the config.
/// You can also use the dialog modeless (then you can use wxAPPLY
/// to store the items in the config).
/// When pressing the apply button ConfigDialogApplied is invoked from exFrame.
/// If you only specify a wxCANCEL button, the dialog is readonly.
class exConfigDialog: public exDialog
{
public:
  /// Constructor, specify the vector of config items
  /// to be used. When wxOK or wxAPPLY is pressed, any change in one of the
  /// config items is saved in the config.
  /// \todo The dialog does not set it's window size correctly when
  /// notebooks are used, you have to specify size yourself.
  exConfigDialog(wxWindow* parent,
    std::vector<exConfigItem> v,
    const wxString& title = _("Options"),
    const wxString& configGroup = wxEmptyString,
    int rows = 0,
    int cols = 1,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  wxControl* Add(
    wxSizer* sizer,
    wxWindow* parent,
    wxControl* control,
    const wxString& text,
    bool expand = true);
  wxControl* AddCheckBox(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text);
  wxControl* AddCheckListBox(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text,
    std::map<int, const wxString> & choices);
  wxControl* AddColourButton(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text);
  wxControl* AddComboBox(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text);
  wxControl* AddComboBoxDir(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text);
  wxControl* AddDirPickerCtrl(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text);
  wxControl* AddFilePickerCtrl(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text);
  wxControl* AddFontPickerCtrlCtrl(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text);
  wxControl* AddRadioBox(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text,
    std::map<int, const wxString> & choices);
  wxControl* AddSpinCtrl(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text,
    int min,
    int max);
  wxControl* AddTextCtrl(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text,
    bool is_numeric = false,
    long style = 0);

  exConfig* m_Config;
  const wxString m_ConfigGroup;
  std::vector<exConfigItem> m_ConfigItems;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI

/// Adds an existing config to wxFindReplaceData, and some members.
class exFindReplaceData : public wxFindReplaceData
{
public:
  /// Constructor, gets members from config.
  exFindReplaceData(exConfig* config);

  /// Destructor, saves members to config.
 ~exFindReplaceData();

  /// Gets the regular expression.
  const wxRegEx& GetFindRegularExpression() const {
    return m_FindRegularExpression;};

  /// Gets the case insensitive find string.
  const wxString& GetFindStringNoCase() const {
    return m_FindStringNoCase;};

  /// Returns true if the flags are used as regular expression.
  /// \todo Add separate member.
  bool IsRegExp() const;

  /// Returns true if the flags have match case set.
  bool MatchCase() const {return (GetFlags() & wxFR_MATCHCASE) > 0;};

  /// Returns true if the flags have whole word set.
  bool MatchWord() const {return (GetFlags() & wxFR_WHOLEWORD) > 0;};

  /// Sets the find string.
  /// If IsRegExp also sets the  and regular expression, if it returns false,
  /// no valid regular expression has been entered.
  /// This string is used for tool find in files and replace in files.
  bool SetFindString(const wxString& value);

  /// Sets flags for match case.
  void SetMatchCase(bool value);

  /// Sets flags for match word.
  void SetMatchWord(bool value);

  /// Updates data from config.
  void Update();
private:
  exConfig* m_Config;
  wxRegEx m_FindRegularExpression;
  wxString m_FindStringNoCase; // same as the FindString, but case insensitive
};
#endif
