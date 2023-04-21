////////////////////////////////////////////////////////////////////////////////
// Name:      factory/beautify.h
// Purpose:   Declaration of wex::factory::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/config.h>
#include <wex/core/path.h>

#include <string>

namespace wex
{
namespace factory
{
/// Offers functionality to beautify source code.
class beautify
{
public:
  /// Beautifies the specified file
  /// (the auto beautifier should explicitly be enabled).
  /// Return false if it did not succeed.
  bool file(const path& p) const;

  /// Returns true if beautifier is set non-empty in the config.
  bool is_active() const;

  /// Returns true if auto beautifier is set in the config.
  bool is_auto() const;

  /// Returns true if specified path can be beautified.
  bool is_supported(const path& p) const;

  /// Returns default beautifiers.
  config::strings_t list() const;

  /// Returns the actual beautifier, or empty string if none selected.
  const std::string name() const;
};
}; // namespace factory
}; // namespace wex
