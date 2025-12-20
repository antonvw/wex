////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff-parser.h
// Purpose:   Declaration of class wex::factory::unified_diff_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
namespace factory
{

class unified_diff;

/// Offers a class that parses a unified diff string and report diffs
/// for a derived class.
/// Context is not expected, you have to create a diff using
/// -U0 (no context).
class unified_diff_parser
{
public:
  /// Constructor, specify the unified_diff.
  unified_diff_parser(unified_diff* diff);

  /// Parse the unified diff input into the unified_diff object.
  /// Returns true and fills diff data if input can be parsed correctly.
  bool parse(bool trace_mode = false);

private:
  unified_diff* m_diff;
};
}; // namespace factory
}; // namespace wex
