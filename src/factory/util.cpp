////////////////////////////////////////////////////////////////////////////////
// Name:      factory/util.cpp
// Purpose:   Implementation of wex factory utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/frame.h>
#include <wex/factory/util.h>
#include <wx/app.h>

void wex::bind_set_focus(wxEvtHandler* handler)
{
  auto* frame = dynamic_cast<factory::frame*>(wxTheApp->GetTopWindow());

  handler->Bind(
    wxEVT_SET_FOCUS,
    [&, frame](wxFocusEvent& event)
    {
      frame->set_find_focus(frame);
      event.Skip();
    });
}

void wex::node_properties(
  const pugi::xml_node*  node,
  std::vector<property>& properties)
{
  for (const auto& child : node->children())
  {
    if (strcmp(child.name(), "property") == 0)
    {
      properties.emplace_back(child);
    }
  }
}

void wex::node_styles(
  const pugi::xml_node* node,
  const std::string&    lexer,
  std::vector<style>&   styles)
{
  for (const auto& child : node->children())
  {
    if (strcmp(child.name(), "style") == 0)
    {
      styles.emplace_back(child, lexer);
    }
  }
}
