////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of syntax util functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/property.h>
#include <wex/syntax/style.h>

namespace wex
{
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
