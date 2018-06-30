////////////////////////////////////////////////////////////////////////////////
// Name:      test-configitem.h
// Purpose:   Declaration and implementation of TestConfigItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
  wxTextValidator* validator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
  validator->SetCharIncludes("0123");
  
  return std::vector<wxExItem> {
    {"notebook", {
      {"Buttons",
        {{"<span size='x-large' color='blue'>Big</span> <b>bold</b> button", ITEM_BUTTON},
         {"lambda", ITEM_BUTTON, std::any(), wxExControlData(), LABEL_LEFT,
           [=](wxWindow* user, const std::any& value, bool save) {
             wxLogStatus("click on lambda");}}}},
      {"Checkboxes",
        {{"Checkbox", ITEM_CHECKBOX}, 
         {"Group Checkbox1", ITEM_CHECKBOX}, 
         {"Group Checkbox2", ITEM_CHECKBOX}}}, 
      {"Checkbox lists",
        {{"Bin Choices", {
           {1, "Bit One"}, 
           {2, "Bit Two"}, 
           {4, "Bit Three"}, 
           {8, "Bit Four"}},
           false}, 
         {{"This", "Or", "Other", "a", "b", "c", "d", "e", "f", "g", "h"}}}},
      {"Colours",
        {{"Colour1", ITEM_COLOURPICKERWIDGET}}}, 
      {"Comboboxes",
        {{"Combobox", ITEM_COMBOBOX}, 
         {"Combobox No Label", ITEM_COMBOBOX},
         {"Combobox Dir Required", ITEM_COMBOBOX_DIR, std::any(), wxExControlData().Required(true)},
         {"Combobox Dir", ITEM_COMBOBOX_DIR},
         {"Combobox File", ITEM_COMBOBOX_FILE}}},
      {"Command Link Buttons",
        {{"Command Link Button\tThis text describes what the button does", ITEM_COMMANDLINKBUTTON}}},
      {"Pickers",
        {{10},
         {"Dir Picker", ITEM_DIRPICKERCTRL}, 
         {"File Picker", ITEM_FILEPICKERCTRL}, 
         {"Font Picker", ITEM_FONTPICKERCTRL}}}, 
      {"Floats", 
        {{"Float", ITEM_TEXTCTRL_FLOAT}}},
      {"Hyperlinks", 
        {{"Hyper Link 1", "www.wxwidgets.org", ITEM_HYPERLINKCTRL},
         {"Hyper Link 2", "www.scintilla.org", ITEM_HYPERLINKCTRL}}},
      {"Integers", 
        {{"Integer", ITEM_TEXTCTRL_INT}}},
      {"ListView",
        {{"ListView", wxExListViewData()}}},
      {"Radioboxes",
        {{"Radio Box", {
           {0, "Zero"}, {1, "One"}, {2, "Two"}, {3, "Three"}, {4, "Four"}, {5, "Five"}},
           true}}}, 
      {"Spin controls",
        {{"Slider", 1, 3, 2, ITEM_SLIDER},
         {"Spin Control", 1, 2}, 
         {"Spin Control Double", 1.0, 3.0, 1.0, 0.01}}},
      {"Static Text",
        {{"Static Text", "this is my static text", ITEM_STATICTEXT}}},
      {"Static Line", 
        {{wxHORIZONTAL},
         {wxVERTICAL}}},
      {"STC",
        {{"STC", "cpp", ITEM_STC}}},
      {"Strings",
        {{"String"},
         {"String Validator", std::string(), ITEM_TEXTCTRL, wxExControlData().Validator(validator)},
         {"String Multiline", std::string(), ITEM_TEXTCTRL, wxExControlData().Window(wxExWindowData().Style(wxTE_MULTILINE))}}},
      {"Toggle buttons",
        {{"Toggle Button", ITEM_TOGGLEBUTTON}}},
      {"User Controls",
        {{"HTML Control", 
           new wxHtmlWindow(),
           [=](wxWindow* user, wxWindow* parent, bool readonly) {
             ((wxHtmlWindow *)user)->Create(parent, 100, wxDefaultPosition, wxSize(200, 200));
             ((wxHtmlWindow *)user)->SetPage("<html><body><b>Hello</b>, <i>world!</i></body></html>");},
           nullptr},
         {"Text Control", 
           new wxTextCtrl(),
           [=](wxWindow* user, wxWindow* parent, bool readonly) {
             ((wxTextCtrl *)user)->Create(parent, 100, "Hello world");},
           [=](wxWindow* user, bool save) {
             if (save) wxConfigBase::Get()->Write("mytext", ((wxTextCtrl *)user)->GetValue());
             return true;},
           LABEL_LEFT,
           [=](wxWindow* user, const std::any& value, bool save) {
             wxLogStatus(((wxTextCtrl *)user)->GetValue());
             }}}}}, 
    ITEM_NOTEBOOK_LIST, rows, cols}};
}
