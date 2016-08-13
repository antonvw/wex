////////////////////////////////////////////////////////////////////////////////
// Name:      test-configitem.h
// Purpose:   Declaration and implementation of TestConfigItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/html/htmlwin.h>
#include <wx/extension/item.h>

/// Returns a vector with all config items available.
/// The first item is a notebook, containing the other items,
/// the arguments are arguments for the notebook item.
const auto TestConfigItems(
  int rows = 0,
  int cols = 0)
{
  wxExItem ci("String Validator");
  
  wxTextValidator* validator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
  validator->SetCharIncludes("0123");
  ci.SetValidator(validator);
  
  return std::vector<wxExItem> {
    wxExItem("notebook", {
      {"Buttons",
        {wxExItem("<span size='x-large' color='blue'>Big</span> <b>bold</b> button",
           ITEM_BUTTON, wxAny(), false, 1000),
         wxExItem("lambda",
           ITEM_BUTTON, wxAny(), false, 1003, LABEL_LEFT, 0,
           [=](wxWindow* user, const wxAny& value, bool save) {
             wxLogStatus("click on lambda");})}},
      {"Checkboxes",
        {wxExItem("Checkbox", ITEM_CHECKBOX), 
         wxExItem("Group Checkbox1", ITEM_CHECKBOX), 
         wxExItem("Group Checkbox2", ITEM_CHECKBOX)}}, 
      {"Checkbox lists",
        {wxExItem("Bin Choices", {
           {1, "Bit One"}, {2, "Bit Two"}, {4, "Bit Three"}, {8, "Bit Four"}},
           false), 
         wxExItem({"This", "Or", "Other", "a", "b", "c", "d", "e", "f", "g", "h"})}},
      {"Colours",
        {wxExItem("Colour1", ITEM_COLOURPICKERWIDGET)}}, 
      {"Comboboxes",
        {wxExItem("Combobox", ITEM_COMBOBOX), 
         wxExItem("Combobox No Label", ITEM_COMBOBOX, 
           wxAny(), false, wxWindow::NewControlId(), LABEL_NONE),
         wxExItem("Combobox Dir Required", ITEM_COMBOBOX_DIR, wxAny(), true, 1011),
         wxExItem("Combobox Dir", ITEM_COMBOBOX_DIR, wxAny(), false, 1012)}},
      {"Command Link Buttons",
        {wxExItem("Command Link Button\tThis text describes what the button does",
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
        {wxExItem("Radio Box", {
           {0, "Zero"}, {1, "One"}, {2, "Two"}, {3, "Three"}, {4, "Four"}, {5, "Five"}},
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
        {wxExItem("HTML Control", 
           new wxHtmlWindow(),
           [=](wxWindow* user, wxWindow* parent, bool readonly) {
             ((wxHtmlWindow *)user)->Create(parent, 100);},
           nullptr),
         wxExItem("Text Control", 
           new wxTextCtrl(),
           [=](wxWindow* user, wxWindow* parent, bool readonly) {
             ((wxTextCtrl *)user)->Create(parent, 100, "hello world");},
           [=](wxWindow* user, bool save) {
             if (save) wxConfigBase::Get()->Write("mytext", ((wxTextCtrl *)user)->GetValue());
             return true;},
           false,
           LABEL_LEFT,
           [=](wxWindow* user, const wxAny& value, bool save) {
             wxLogStatus(((wxTextCtrl *)user)->GetValue());
             })}}}, 
    ITEM_NOTEBOOK_LIST, 0, rows, cols)};
}
