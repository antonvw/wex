////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of factory util functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/property.h>
#include <wex/factory/style.h>

namespace wex
{
/// Binds to focus.
void bind_set_focus(wxEvtHandler* handler);

/// Parses properties node.
void node_properties(
  const pugi::xml_node*  node,
  std::vector<property>& properties);

/// Parses style node.
void node_styles(
  const pugi::xml_node* node,
  const std::string&    lexer,
  std::vector<style>&   styles);
}; // namespace wex
