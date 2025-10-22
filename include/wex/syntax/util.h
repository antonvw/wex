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
bool single_choice_dialog(
  /// all window data
  const data::window& data,
  /// the strings to choose from
  const std::vector<std::string>& v,
  /// the selected one
  std::string& selection);
}; // namespace wex
