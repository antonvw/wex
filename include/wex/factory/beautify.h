////////////////////////////////////////////////////////////////////////////////
// Name:      factory/beautify.h
// Purpose:   Declaration of wex::factory::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2024 Anton van Wezenbeek
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
  /// The supported beautify types.
  enum beautify_t
  {
    CMAKE,          ///< cmake
    ROBOTFRAMEWORK, ///< robotframework
    SOURCE,         ///< source code (c, c#)
    UNKNOWN,        ///< type will be set later on
  };

  /// Default constructor, specify the beautify type.
  beautify(beautify_t = UNKNOWN);

  /// Constructor, the path is used to set the beautify type.
  beautify(const path& p);

  /// Checks if a beautifier exists for specified path
  /// and sets type if so.
  bool check(const path& p);

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

  /// Returns the beautify type.
  beautify_t type() const { return m_type; };

private:
  beautify_t m_type{UNKNOWN};
};
}; // namespace factory
}; // namespace wex
