////////////////////////////////////////////////////////////////////////////////
// Name:      test-configitem.h
// Purpose:   Declaration and implementation of TestConfigItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/html/htmlwin.h>
#include <wx/extension/configitem.h>

void myHtmlCreate(wxWindow* user, wxWindow* parent, bool readonly)
{
  ((wxHtmlWindow *)user)->Create(parent, 100);
}

void myTextCreate(wxWindow* user, wxWindow* parent, bool readonly)
{
  ((wxTextCtrl *)user)->Create(parent, 100);
}

/// Returns a vector with all config items available.
const auto TestConfigItems()
{
  std::vector<wxExConfigItem> v;

  // ITEM_BUTTON
  for (int b = 1; b <= 4; b++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("<span size='x-large' color='blue'>Big</span> <b>bold</b> button %d", b),
      ITEM_BUTTON,
      "Buttons",
      false,
      1000 + b));
  }

  // ITEM_CHECKBOX
  for (int h = 1; h <= 4; h++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Checkbox%d", h), 
      ITEM_CHECKBOX, 
      "Checkboxes"));
  }

  v.push_back(wxExConfigItem(
    "Group Checkbox1",
    ITEM_CHECKBOX, 
    "Checkboxes"));

  v.push_back(wxExConfigItem(
    "Group Checkbox2",
    ITEM_CHECKBOX, 
    "Checkboxes"));

  // ITEM_CHECKLISTBOX
  v.push_back(wxExConfigItem(
    "Bin Choices", 
    std::map<long, const wxString> {
      std::make_pair(1, "Bit One"),
      std::make_pair(2, "Bit Two"),
      std::make_pair(4, "Bit Three"),
      std::make_pair(8, "Bit Four")},
    false, 
    "Checkbox lists"));

  // ITEM_CHECKLISTBOX_BOOL
  v.push_back(wxExConfigItem(
    std::set<wxString>{"This","Or","Other", "a", "b", "c", "d",
    "e", "f", "g", "h"},
    "Checkbox lists"));

  // ITEM_COLOUR
  for (int i = 1; i <= 5; i++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Colour%d", i), 
      ITEM_COLOUR, 
      "Colours"));
  }

  // ITEM_COMBOBOX
  for (int m = 1; m <= 5; m++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Combobox%d", m), 
      ITEM_COMBOBOX, 
      "Comboboxes"));
  }

  // ITEM_COMBOBOX without a name
  v.push_back(wxExConfigItem(
    "Combobox No Name",
    ITEM_COMBOBOX, 
    "Comboboxes",
    false,
    wxID_ANY,
    wxWindow::NewControlId(),
    false));

  // ITEM_COMBOBOXDIR
  v.push_back(wxExConfigItem(
    "Combobox Dir Required",
    ITEM_COMBOBOXDIR, 
    "Comboboxes",
    true,
    wxWindow::NewControlId()));

  // ITEM_COMBOBOXDIR
  v.push_back(wxExConfigItem(
    "Combobox Dir", 
    ITEM_COMBOBOXDIR, 
    "Comboboxes",
    false,
    wxWindow::NewControlId()));

  // ITEM_COMMAND_LINK_BUTTON
  for (int l = 1; l <= 4; l++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Command Link Button%d\tThis text describes what the button does", l),
      ITEM_COMMAND_LINK_BUTTON,
      "Command Link Buttons",
      false,
      1010 + l));
  }

  // ITEM_SPACER on pickers page
  v.push_back(wxExConfigItem(10, "Pickers"));

  // ITEM_DIRPICKERCTRL
  v.push_back(wxExConfigItem(
    "Dir Picker", 
    ITEM_DIRPICKERCTRL, 
    "Pickers"));

  // ITEM_FILEPICKERCTRL
  v.push_back(wxExConfigItem(
    "File Picker", 
    ITEM_FILEPICKERCTRL, 
    "Pickers"));

  // ITEM_FONTPICKERCTRL
  v.push_back(wxExConfigItem(
    "Font Picker", 
    ITEM_FONTPICKERCTRL, 
    "Pickers"));

  // ITEM_FLOAT
  v.push_back(wxExConfigItem(
    "Float", 
    ITEM_FLOAT, 
    "Floats", 
    true));
      
  // ITEM_HYPERLINKCTRL
  v.push_back(wxExConfigItem(
    "Hyper Link 1",
    "www.wxwidgets.org",
    "Hyperlinks",
    0,
    ITEM_HYPERLINKCTRL));

  v.push_back(wxExConfigItem(
    "Hyper Link 2",
    "www.scintilla.org",
    "Hyperlinks",
    0,
    ITEM_HYPERLINKCTRL));

  // ITEM_INT
  for (int j = 1; j <= 5; j++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Integer%d", j), 
      ITEM_INT, 
      "Integers", 
      true));
  }

  // ITEM_LISTVIEW_FOLDER
  v.push_back(wxExConfigItem(
    "ListView",
    ITEM_LISTVIEW_FOLDER,
    "ListView"));

  // ITEM_RADIOBOX
  v.push_back(wxExConfigItem(
    "Radio Box", 
    std::map<long, const wxString> {
      std::make_pair(0, "Zero"),
      std::make_pair(1, "One"),
      std::make_pair(2, "Two"),
      std::make_pair(3, "Three"),
      std::make_pair(4, "Four"),
      std::make_pair(5, "Five")},
    true, 
    "Radioboxes"));

  // ITEM_SLIDER
  const int start = 1;
  for (int sl = start + 1; sl <= start + 3; sl++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Slider%d", sl),
      start,
      sl,
      "Spin controls",
      ITEM_SLIDER));
  }

  // ITEM_SPINCTRL
  for (int s = 1; s <= 2; s++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Spin Control%d", s), 
      1, 
      s, 
      "Spin controls"));
  }

  // ITEM_SPINCTRL_DOUBLE
  for (int sd = 1; sd <= 2; sd++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Spin Control Double%d", sd), 
      1.0,
      (double)sd, 
      "Spin controls",
      0.01));
  }

  // ITEM_SPINCTRL_HEX
  for (int s = 1; s <= 2; s++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Spin Control Hex%d", s), 
      0, 
      255, 
      "Spin controls",
      ITEM_SPINCTRL_HEX));
  }

  for (int st = 1; st <= 5; st++)
  {
    // ITEM_STATICTEXT
    v.push_back(wxExConfigItem(
      wxString::Format("Static Text%d", st),
      "this is my static text",
      "Static Text",
      0,
      ITEM_STATICTEXT));
  }

  // ITEM_STATICLINE (horizontal)
  v.push_back(wxExConfigItem(wxHORIZONTAL, "Static Line"));

  // ITEM_STATICLINE (vertical)
  v.push_back(wxExConfigItem(wxVERTICAL, "Static Line"));

  // ITEM_STC
  v.push_back(wxExConfigItem(
    "STC", 
    "cpp",
    "STC",
    0,
    ITEM_STC));

  // ITEM_STRING
  for (int l = 1; l <= 5; l++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("String%d", l), 
      wxEmptyString,
      "Strings"));
  }

  wxExConfigItem ci(
    "String Validator", 
    wxEmptyString,
    "Strings");
  wxTextValidator* validator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
  validator->SetCharIncludes("0123");
  ci.SetValidator(validator);
  v.push_back(ci);

  v.push_back(wxExConfigItem(
    "String Multiline", 
    wxEmptyString,
    "Strings",
    wxTE_MULTILINE));

  // ITEM_TOGGLEBUTTON
  for (int tb = 1; tb <= 4; tb++)
  {
    v.push_back(wxExConfigItem(
      wxString::Format("Toggle Button%d", tb),
      ITEM_TOGGLEBUTTON,
      "Toggle buttons",
      false,
      1020 + tb));
  }

  /// ITEM_USER
  v.push_back(wxExConfigItem(
    "HTML Control", 
    new wxHtmlWindow(),
    myHtmlCreate,
    NULL,
    "User Controls"));

  v.push_back(wxExConfigItem(
    "Text Control", 
    new wxTextCtrl(),
    myTextCreate,
    NULL,
    "User Controls"));
  
  return v;
}
