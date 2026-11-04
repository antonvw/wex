////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wex syntax utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/stc.h>
#include <wex/syntax/util.h>

int wex::find_xml_root(wxStyledTextCtrl* stc, size_t skip)
{
  stc->SetSearchFlags(wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX);

  int start_pos = 0;

  for (int i = 0; i <= skip; i++)
  {
    stc->SetTargetRange(start_pos, 1000);

    start_pos = stc->SearchInTarget("<[^?].*>");

    if (start_pos == -1)
    {
      return 0;
    }

    start_pos++;
  }

  return stc->LineFromPosition(start_pos);
}

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
