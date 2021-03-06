////////////////////////////////////////////////////////////////////////////////
// Name:      beautify.h
// Purpose:   Declaration of wex::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wex/config.h>
#include <wex/lexer.h>
#include <wex/path.h>
#include <wex/stc.h>

namespace wex
{
/// This class defines a class that allows to beautify source code.
class beautify
{
public:
  /// Beautifies the specified file.
  /// Return false if it did not succeed.
  bool file(const path& p) const;

  /// Returns true if beautifier is set non-empty in the config.
  bool is_active() const;

  /// Returns true if auto beautifier is set in the config.
  bool is_auto() const;

  /// Returns true if specified lexer can be beautified.
  bool is_supported(const lexer& l) const;

  /// Returns default beautifiers.
  config::strings_t list() const;

  /// Beautifies the specified stc component.
  /// Return false if it did not succeed.
  bool stc(wex::stc& s) const;

private:
  /// Returns the actual beautifier, or empty string if none
  /// selected.
  const std::string name() const;
};
}; // namespace wex
