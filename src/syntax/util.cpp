////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wex syntax utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/util.h>
#include <wx/choicdlg.h>

void wex::node_properties(
  const pugi::xml_node*  node,
  std::vector<property>& properties)
{
  for (const auto& child : node->children("property"))
  {
    properties.emplace_back(child);
  }
}

void wex::node_styles(
  const pugi::xml_node* node,
  const std::string&    lexer,
  std::vector<style>&   styles)
{
  for (const auto& child : node->children("style"))
  {
    styles.emplace_back(child, lexer);
  }
}

bool wex::single_choice_dialog(
  const data::window&             data,
  const std::vector<std::string>& v,
  std::string&                    selection)
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
