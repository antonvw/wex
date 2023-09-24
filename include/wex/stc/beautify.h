////////////////////////////////////////////////////////////////////////////////
// Name:      beautify.h
// Purpose:   Declaration of wex::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/beautify.h>
#include <wex/stc/stc.h>
#include <wex/syntax/lexer.h>

namespace wex
{
/// This class adds functionality to factory to beautify source code from stc.
class beautify : public factory::beautify
{
public:
  /// Returns true if specified lexer can be beautified.
  bool is_supported(const lexer& l) const;

  /// Beautifies the specified stc component
  /// Return false if it did not succeed.
  bool stc(wex::stc& s) const;
};
}; // namespace wex
