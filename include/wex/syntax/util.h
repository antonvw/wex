////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of syntax util functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/window.h>
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

/// Presents a dialog to choose one string out of an array.
bool single_choice_dialog(
  /// all window data
  const data::window& data,
  /// the strings to choose from
  const std::vector<std::string>& v,
  /// the selected one
  std::string& selection);
}; // namespace wex
