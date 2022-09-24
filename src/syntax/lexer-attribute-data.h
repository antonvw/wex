////////////////////////////////////////////////////////////////////////////////
// Name:      lexer-attribute-data.h
// Purpose:   Declaration of wex::lexer_attribute_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/lexer.h>

namespace pugi
{
class xml_attribute;
class xml_node;
}; // namespace pugi

namespace wex
{
/// This class offers lexer xml attribute support.
/// Currently it parses the keyword sets that are referenced in the
/// attributes.
class lexer_attribute_data
{
public:
  /// Constructor, specify node and a single attribute.
  /// The attribute can specify: set(-<number>)?=<lexers-set>(,<lexers-set>)*
  /// E.g.:
  /// set="cisco-0"
  /// set-1="cisco-1"
  /// set-2="cisco-2,cisco-3,cisco-4"
  lexer_attribute_data(const pugi::xml_node* n, const pugi::xml_attribute& a);

  /// Adds all keywords from the sets to the specified lexer.
  void add_keywords(lexer& l) const;

private:
  const pugi::xml_attribute m_att;
  const pugi::xml_node*     m_node;

  int m_setno{0};
};
}; // namespace wex
