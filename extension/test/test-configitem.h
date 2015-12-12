////////////////////////////////////////////////////////////////////////////////
// Name:      test-configitem.h
// Purpose:   Declaration and implementation of TestConfigItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/html/htmlwin.h>
#include <wx/extension/item.h>

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
  wxExItem ci("String Validator");
  
  wxTextValidator* validator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
  validator->SetCharIncludes("0123");
  ci.SetValidator(validator);
  
  return std::vector<wxExItem> {
    wxExItem("notebook", wxExItem::ItemsNotebook {
      {"Buttons",
      {wxExItem("<span size='x-large' color='blue'>Big</span> <b>bold</b> button",
         ITEM_BUTTON,
         wxAny(),
         false,
         1000)}},
      {"Checkboxes",
      {wxExItem("Checkbox", ITEM_CHECKBOX), 
       wxExItem("Group Checkbox1", ITEM_CHECKBOX), 
       wxExItem("Group Checkbox2", ITEM_CHECKBOX)}}, 
      {"Checkbox lists",
      {wxExItem("Bin Choices", 
         std::map<long, const wxString> {
           {1, "Bit One"},
           {2, "Bit Two"},
           {4, "Bit Three"},
           {8, "Bit Four"}},
           false), 
       wxExItem(std::set<wxString>{"This","Or","Other", "a", "b", "c", "d",
         "e", "f", "g", "h"})}},
      {"Colours",
      {wxExItem("Colour1", ITEM_COLOURPICKERWIDGET)}}, 
      {"Comboboxes",
      {wxExItem("Combobox", ITEM_COMBOBOX), 
       wxExItem("Combobox No Label", ITEM_COMBOBOX, 
         wxAny(), false, wxWindow::NewControlId(), LABEL_NONE),
       wxExItem("Combobox Dir Required", ITEM_COMBOBOX_DIR, wxAny(), true),
       wxExItem("Combobox Dir", ITEM_COMBOBOX_DIR)}},
      {"Command Link Buttons",
      {wxExItem("Command Link Button%d\tThis text describes what the button does",
         ITEM_COMMANDLINKBUTTON, wxAny(), false, 1010)}},
      {"Pickers",
      {wxExItem(10),
       wxExItem("Dir Picker", ITEM_DIRPICKERCTRL), 
       wxExItem("File Picker", ITEM_FILEPICKERCTRL), 
       wxExItem("Font Picker", ITEM_FONTPICKERCTRL)}}, 
      {"Floats", 
      {wxExItem("Float", ITEM_TEXTCTRL_FLOAT)}},
      {"Hyperlinks", 
      {wxExItem("Hyper Link 1", "www.wxwidgets.org", 0, ITEM_HYPERLINKCTRL),
       wxExItem("Hyper Link 2", "www.scintilla.org", 0, ITEM_HYPERLINKCTRL)}},
      {"Integers", 
      {wxExItem("Integer", ITEM_TEXTCTRL_INT)}},
      {"ListView",
      {wxExItem("ListView", ITEM_LISTVIEW)}},
      {"Radioboxes",
      {wxExItem("Radio Box", 
         std::map<long, const wxString> {
           {0, "Zero"},
           {1, "One"},
           {2, "Two"},
           {3, "Three"},
           {4, "Four"},
           {5, "Five"}},
         true)}}, 
      {"Spin controls",
      {wxExItem("Slider", 1, 3, 2, ITEM_SLIDER),
       wxExItem("Spin Control", 1, 2), 
       wxExItem("Spin Control Double", 1.0, 3.0, 1.0, 0.01)}},
      {"Static Text",
      {wxExItem("Static Text", "this is my static text",
         0,
         ITEM_STATICTEXT)}},
      {"Static Line",
      {wxExItem(wxHORIZONTAL),
       wxExItem(wxVERTICAL)}},
      {"STC",
      {wxExItem("STC", "cpp", 0, ITEM_STC)}},
      {"Strings",
      {wxExItem("String"),
       ci,
       wxExItem("String Multiline", wxEmptyString, wxTE_MULTILINE)}},
      {"Toggle buttons",
      {wxExItem("Toggle Button", ITEM_TOGGLEBUTTON, wxAny(), false, 1020 )}},
      {"User Controls",
      {wxExItem("HTML Control", new wxHtmlWindow(),
         myHtmlCreate,
         nullptr),
       wxExItem("Text Control", new wxTextCtrl(),
         myTextCreate,
         nullptr)}}}, ITEM_NOTEBOOK_LIST, 0, 0, 1)};
}
