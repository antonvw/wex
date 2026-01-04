////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff-parser.h
// Purpose:   Declaration of class wex::factory::unified_diff_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
namespace factory
{

class unified_diff;

/// Offers a class that implements the parsing for the unified diff class.
class unified_diff_parser
{
public:
  /// Constructor, specify the unified_diff.
  unified_diff_parser(unified_diff* diff);

  /// Parse the unified diff input into the unified_diff object.
  /// Returns true and fills diff data if input is parsed correctly.
  bool parse();

private:
  unified_diff* m_diff;
};
}; // namespace factory
}; // namespace wex
