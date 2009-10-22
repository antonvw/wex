/******************************************************************************\
* File:          configdlg.h
* Purpose:       Declaration of wxExtension config dialog classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXCONFIGDIALOG_H
#define _EXCONFIGDIALOG_H

#include <map>
#include <set>
#include <vector>
#include <wx/config.h>
#include <wx/extension/dialog.h>

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

  CONFIG_CHECKBOX,       ///< a checkbox (use ReadBool to retrieve value)
  CONFIG_COLOUR,         ///< a colour button
  CONFIG_COMBOBOX,       ///< a combobox
  CONFIG_COMBOBOXDIR,    ///< a combobox with a browse button
  CONFIG_DIRPICKERCTRL,  ///< a dirpicker ctrl
  CONFIG_FILEPICKERCTRL, ///< a filepicker ctrl
  CONFIG_FONTPICKERCTRL, ///< a fontpicker ctrl
  CONFIG_INT,            ///< a textctrl that only accepts an integer (a long integer)
  CONFIG_SPINCTRL,       ///< a spinctrl
  CONFIG_SPINCTRL_DOUBLE, ///< a spinctrl double
  CONFIG_SPACER,         ///< a spacer only, no config item
  CONFIG_STRING,         ///< a textctrl
};

/// Container class for using with wxExConfigDialog.
/// If you specify a page, then all items are placed on that page in a notebook.
class wxExConfigItem
{
  friend class wxExConfigDialog;
public:
  /// Contructor for a spacer item.
  wxExConfigItem()
  : m_Name("spacer")
  , m_Page(wxEmptyString)
  , m_Type(CONFIG_SPACER) {;}

  /// Constructor for a spin ctrl.
  wxExConfigItem(const wxString& name,
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

  /// Constructor for a spin ctrl double.
  wxExConfigItem(const wxString& name,
    double min,
    double max,
    double inc = 1,
    const wxString& page = wxEmptyString)
  : m_IsRequired(false)
  , m_MaxItems(0)
  , m_MinDouble(min)
  , m_MaxDouble(max)
  , m_Inc(inc)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_SPINCTRL_DOUBLE) {;};

  /// Constructor for a string.
  /// The extra style argument is the style for the wxTextCtrl used.
  /// (e.g. wxTE_MULTILINE or wxTE_PASSWORD)
  wxExConfigItem(const wxString& name,
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
  wxExConfigItem(const wxString& name,
    const std::map<long, const wxString> & choices,
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

  /// Constructor for a checklistbox without a name. Just specify
  /// the map with values and text.
  wxExConfigItem(const std::set<wxString> & choices,
    const wxString& page = wxEmptyString)
  : m_IsRequired(false)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name("checklistbox_noname")
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_CHECKLISTBOX_NONAME)
  , m_ChoicesBool(choices) {;};

  /// Constuctor for other types.
  wxExConfigItem(const wxString& name,
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

#if wxUSE_GUI
/// Offers a dialog to set several items in the config.
/// You can also use the dialog modeless (then you can use wxAPPLY
/// to store the items in the config).
/// When pressing the apply button ConfigDialogApplied is invoked from wxExFrame.
/// If you only specify a wxCANCEL button, the dialog is readonly.
class wxExConfigDialog: public wxExDialog
{
public:
  /// Constructor, specify the vector of config items
  /// to be used. When wxOK or wxAPPLY is pressed, any change in one of the
  /// config items is saved in the config.
  /// \todo The dialog does not set it's window size correctly when
  /// notebooks are used, you have to specify size yourself.
  wxExConfigDialog(wxWindow* parent,
    std::vector<wxExConfigItem> v,
    const wxString& title = _("Options"),
    int rows = 0,
    int cols = 2,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = wxDialogNameStr);

  /// If you specified some checkboxes, calling this method
  /// requires that one of them should be checked for OK button
  /// to be enabled.
  void ForceCheckBoxChecked() {m_ForceCheckBoxChecked = true;};
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
    std::map<long, const wxString> & choices);
  wxControl* AddCheckListBoxNoName(
    wxWindow* parent,
    wxSizer* sizer,
    std::set<wxString> & choices);
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
    std::map<long, const wxString> & choices);
  wxControl* AddSpinCtrl(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text,
    int min,
    int max);
  wxControl* AddSpinCtrlDouble(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text,
    double min,
    double max,
    double inc);
  wxControl* AddTextCtrl(
    wxWindow* parent,
    wxSizer* sizer,
    const wxString& text,
    bool is_numeric = false,
    long style = 0);

  wxConfigBase* m_Config;
  std::vector<wxExConfigItem> m_ConfigItems;
  bool m_ForceCheckBoxChecked;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
