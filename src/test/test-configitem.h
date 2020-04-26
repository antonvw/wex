////////////////////////////////////////////////////////////////////////////////
// Name:      test-config_item.h
// Purpose:   Declaration and implementation of test_config_item
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/html/htmlwin.h>
#include <wex/config.h>
#include <wex/item.h>

/// Returns a vector with all config items available.
/// The first item is a notebook, containing the other items,
/// the arguments are arguments for the notebook item.
const auto test_config_items(
  int rows = 0,
  int cols = 0)
{
  auto* validator = new wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
  validator->SetCharIncludes("0123");
  
  return std::vector<wex::item> {
    {"notebook", {
      {"buttons",
        {{"<span size='x-large' color='blue'>Big</span> <b>bold</b> button", 
             wex::item::BUTTON},
         {"lambda", wex::item::BUTTON, std::any(), wex::control_data(), 
             wex::item::LABEL_LEFT,
           [=](wxWindow* user, const std::any& value, bool save) {
             wex::log::status("Click on lambda");}}}},
      {"checkboxes",
        {{"checkbox", wex::item::CHECKBOX}, 
         {"group checkbox1", wex::item::CHECKBOX}, 
         {"group checkbox2", wex::item::CHECKBOX}}}, 
      {"checkbox lists",
        {{"bin choices", {
           {1, "bit one"}, 
           {2, "bit two"}, 
           {4, "bit three"}, 
           {8, "bit four"}},
           false}, 
         {{"This", "Or", "Other", "a", "b", "c", "d", "e", "f", "g", "h"}}}},
      {"colours",
        {{"colour1", wex::item::COLOURPICKERWIDGET}}}, 
      {"comboboxes",
        {{"combobox", wex::item::COMBOBOX}, 
         {"combobox no label", wex::item::COMBOBOX},
         {"combobox dir is_required", 
             wex::item::COMBOBOX_DIR, 
             std::any(), 
             wex::control_data().is_required(true)},
         {"combobox dir", wex::item::COMBOBOX_DIR},
         {"combobox file", wex::item::COMBOBOX_FILE}}},
      {"command link buttons",
        {{"Command Link Button\tThis text describes what the button does", 
             wex::item::COMMANDLINKBUTTON}}},
      {"floats", 
        {{"float", wex::item::TEXTCTRL_FLOAT}}},
      {"grids", 
        {{"grid", wex::item::GRID}}},
      {"hyperlinks", 
        {{"hyper link 1", "www.wxwidgets.org", wex::item::HYPERLINKCTRL},
         {"hyper link 2", "www.scintilla.org", wex::item::HYPERLINKCTRL}}},
      {"integers", 
        {{"integer", wex::item::TEXTCTRL_INT}}},
      {"listviews",
        {{"listview", wex::listview_data()}}},
      {"pickers",
        {{10},
         {"dir picker", wex::item::DIRPICKERCTRL}, 
         {"file picker", wex::item::FILEPICKERCTRL}, 
         {"font picker", wex::item::FONTPICKERCTRL}}}, 
      {"radioboxes",
        {{"radio box", {
            {0, "Zero"}, 
            {1, "One"}, 
            {2, "Two"}, 
            {3, "Three"}, 
            {4, "Four"}, 
            {5, "Five"}},
           true}}}, 
      {"spin controls",
        {{"slider", 1, 3, 2, wex::item::SLIDER},
         {"spin control", 1, 2}, 
         {"spin control double", 1.0, 3.0, 1.0, 0.01}}},
      {"static text",
        {{"static text", "this is my static text", wex::item::STATICTEXT}}},
      {"static line", 
        {{wxHORIZONTAL},
         {wxVERTICAL}}},
      {"stc",
        {{"stc", "cpp", wex::item::STC}}},
      {"strings",
        {{"string"},
         {"string validator", std::string(), 
            wex::item::TEXTCTRL, 
            wex::control_data().validator(validator)},
         {"string multiline", std::string(), 
            wex::item::TEXTCTRL, 
            wex::control_data().window(wex::window_data().style(wxTE_MULTILINE))}}},
      {"toggle buttons",
        {{"toggle button", wex::item::TOGGLEBUTTON}}},
      {"user controls",
        {{"wxHtmlWindow", 
           new wxHtmlWindow(),
           [=](wxWindow* user, wxWindow* parent, bool readonly) {
             ((wxHtmlWindow *)user)->Create(
                parent, 100, wxDefaultPosition, wxSize(200, 200));
             ((wxHtmlWindow *)user)->SetPage(
                "<html><body><b>Hello</b>, <i>world!</i></body></html>");},
           nullptr},
         {"wxTextCtrl", 
           new wxTextCtrl(),
           [=](wxWindow* user, wxWindow* parent, bool readonly) {
             ((wxTextCtrl *)user)->Create(parent, 100, "Hello world");},
           [=](wxWindow* user, bool save) {
             if (save) wex::config("mytext").set(
                  ((wxTextCtrl *)user)->GetValue().ToStdString());
             return true;},
           wex::item::LABEL_LEFT,
           [=](wxWindow* user, const std::any& value, bool save) {
             wex::log::status(((wxTextCtrl *)user)->GetValue());
             }}}}}, 
    wex::item::NOTEBOOK_LIST, rows, cols}};
}