/******************************************************************************\
* File:          config.h
* Purpose:       Declaration of wxWidgets config extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: config.h 53 2008-11-13 18:38:57Z anton $
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
    const wxString& key, long default_value);

  /// Gets the key as a string. If the key is not present,
  /// it is added to the map of string values.
  /// This also works for comboboxes,
  /// as long as the values are separated by default row delimiter,
  /// as then it returns value before this delimiter.
  const wxString Get(
    const wxString& key,
    const wxString& default_value = wxEmptyString,
    const wxChar field_separator = ',');

  /// Gets the key as a bool. If the key is not present,
  /// it is added to the map of bool values.
  bool GetBool(
    const wxString& key, bool default_value = true);

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
  CONFIG_CHECKBOX,       ///< a check box (use GetBool to retrieve value)
  CONFIG_COLOUR,         ///< a colour button
  CONFIG_COMBOBOX,       ///< a combo box
  CONFIG_COMBOBOXDIR,    ///< a combo box with a browse button
  CONFIG_DIRPICKERCTRL,  ///< a dir picker ctrl
  CONFIG_FILEPICKERCTRL, ///< a file picker ctrl
  CONFIG_FONTPICKERCTRL, ///< a font picker ctrl
  CONFIG_INT,            ///< a text ctrl that only accepts an integer (a long integer)
  CONFIG_SPINCTRL,       ///< a spin ctrl
  CONFIG_STRING,         ///< a text ctrl
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
