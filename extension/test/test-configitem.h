////////////////////////////////////////////////////////////////////////////////
// Name:      test-config_item.h
// Purpose:   Declaration and implementation of TestConfigItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/html/htmlwin.h>
#include <wex/config.h>
#include <wex/item.h>

/// Returns a vector with all config items available.
/// The first item is a notebook, containing the other items,
/// the arguments are arguments for the notebook item.
const auto TestConfigItems(
  int rows = 0,
  int cols = 0)
{
  wxTextValidator* validator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
  validator->SetCharIncludes("0123");
  
  return std::vector<wex::item> {
    {"notebook", {
      {"Buttons",
        {{"<span size='x-large' color='blue'>Big</span> <b>bold</b> button", wex::item::BUTTON},
         {"lambda", wex::item::BUTTON, std::any(), wex::control_data(), wex::item::LABEL_LEFT,
           [=](wxWindow* user, const std::any& value, bool save) {
             wxLogStatus("click on lambda");}}}},
      {"Checkboxes",
        {{"Checkbox", wex::item::CHECKBOX}, 
         {"Group Checkbox1", wex::item::CHECKBOX}, 
         {"Group Checkbox2", wex::item::CHECKBOX}}}, 
      {"Checkbox lists",
        {{"Bin Choices", {
           {1, "Bit One"}, 
           {2, "Bit Two"}, 
           {4, "Bit Three"}, 
           {8, "Bit Four"}},
           false}, 
         {{"This", "Or", "Other", "a", "b", "c", "d", "e", "f", "g", "h"}}}},
      {"Colours",
        {{"Colour1", wex::item::COLOURPICKERWIDGET}}}, 
      {"Comboboxes",
        {{"Combobox", wex::item::COMBOBOX}, 
         {"Combobox No Label", wex::item::COMBOBOX},
         {"Combobox Dir Required", wex::item::COMBOBOX_DIR, std::any(), wex::control_data().Required(true)},
         {"Combobox Dir", wex::item::COMBOBOX_DIR},
         {"Combobox File", wex::item::COMBOBOX_FILE}}},
      {"Command Link Buttons",
        {{"Command Link Button\tThis text describes what the button does", wex::item::COMMANDLINKBUTTON}}},
      {"Pickers",
        {{10},
         {"Dir Picker", wex::item::DIRPICKERCTRL}, 
         {"File Picker", wex::item::FILEPICKERCTRL}, 
         {"Font Picker", wex::item::FONTPICKERCTRL}}}, 
      {"Floats", 
        {{"Float", wex::item::TEXTCTRL_FLOAT}}},
      {"Hyperlinks", 
        {{"Hyper Link 1", "www.wxwidgets.org", wex::item::HYPERLINKCTRL},
         {"Hyper Link 2", "www.scintilla.org", wex::item::HYPERLINKCTRL}}},
      {"Integers", 
        {{"Integer", wex::item::TEXTCTRL_INT}}},
      {"ListView",
        {{"ListView", wex::listview_data()}}},
      {"Radioboxes",
        {{"Radio Box", {
           {0, "Zero"}, {1, "One"}, {2, "Two"}, {3, "Three"}, {4, "Four"}, {5, "Five"}},
           true}}}, 
      {"Spin controls",
        {{"Slider", 1, 3, 2, wex::item::SLIDER},
         {"Spin Control", 1, 2}, 
         {"Spin Control Double", 1.0, 3.0, 1.0, 0.01}}},
      {"Static Text",
        {{"Static Text", "this is my static text", wex::item::STATICTEXT}}},
      {"Static Line", 
        {{wxHORIZONTAL},
         {wxVERTICAL}}},
      {"STC",
        {{"STC", "cpp", wex::item::STC}}},
      {"Strings",
        {{"String"},
         {"String Validator", std::string(), wex::item::TEXTCTRL, wex::control_data().Validator(validator)},
         {"String Multiline", std::string(), wex::item::TEXTCTRL, wex::control_data().Window(wex::window_data().Style(wxTE_MULTILINE))}}},
      {"Toggle buttons",
        {{"Toggle Button", wex::item::TOGGLEBUTTON}}},
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
             if (save) wex::config("mytext").set(
                  ((wxTextCtrl *)user)->GetValue().ToStdString());
             return true;},
           wex::item::LABEL_LEFT,
           [=](wxWindow* user, const std::any& value, bool save) {
             wxLogStatus(((wxTextCtrl *)user)->GetValue());
             }}}}}, 
    wex::item::NOTEBOOK_LIST, rows, cols}};
}
