////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of syntax util functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/window.h>
#include <wex/syntax/property.h>
#include <wex/syntax/style.h>
#include <wx/choicdlg.h>

namespace wex
{
/// Returns line containing the xml root in stc document, or 0 if none.
int find_xml_root(
  /// the stc component
  wxStyledTextCtrl* stc,
  /// number of roots to skip
  size_t skip = 0);

/// Applies all styles in the set to the given stc.
/// It is required that T offers an apply function taking wxStyledTextCtrl* as
/// parameter.
template <typename T>
void for_each_style(const T& styles, wxStyledTextCtrl* stc)
{
  std::ranges::for_each(
    styles,
    [stc](const auto& s)
    {
      s.apply(stc);
    });
};

/// Parses properties node.
void node_properties(
  const pugi::xml_node*  node,
  std::vector<property>& properties);

/// Parses style node.
void node_styles(
  const pugi::xml_node* node,
  const std::string&    lexer,
  std::vector<style>&   styles);

/// Presents a dialog to choose one string out of an array.
/// The type T should be a container with std::string.
template <typename T>
bool single_choice_dialog(
  /// all window data
  const data::window& data,
  /// the strings to choose from
  const T& v,
  /// the selected one
  std::string& selection)
{
  wxArrayString s;

  for (const auto& it : v)
  {
    s.Add(it);
  }

  wxSingleChoiceDialog dlg(data.parent(), _("Input") + ":", data.title(), s);

  if (data.size() != wxDefaultSize)
  {
    dlg.SetSize(data.size());
  }

  if (const auto index = s.Index(selection); index != wxNOT_FOUND)
  {
    dlg.SetSelection(index);
  }
  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  selection = dlg.GetStringSelection();

  return true;
}
}; // namespace wex
