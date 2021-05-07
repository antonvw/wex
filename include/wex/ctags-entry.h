////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-entry.h
// Purpose:   Declaration of class wex::ctags_entry
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
/// Offers information about a specific tag (see tagEntry).
class ctags_entry
{
public:
  /// Returns access member.
  const auto& access() const { return m_access; }

  /// Sets access member.
  ctags_entry& access(const std::string& v);

  /// Returns class member.
  const auto& class_name() const { return m_class; }

  /// Sets class member.
  ctags_entry& class_name(const std::string& v);

  /// Clear the member.
  void clear();

  /// Returns true if one of the members is filled.
  bool is_active() const;

  /// Returns kind member.
  const auto& kind() const { return m_kind; }

  /// Sets kind of tag. The value depends on the language.  For C and
  /// C++ these kinds are recommended:
  /// - c	class name
  /// - d	define (from define XXX)
  /// - e	enumerator
  /// - f	function or method name
  /// - F	file name
  /// - g	enumeration name
  /// - m	member (of structure or class data)
  /// - p	function prototype
  /// - s	structure name
  /// - t	typedef
  /// - u	union name
  /// - v	variable
  ctags_entry& kind(const std::string& v);

  /// Logs info about this entry.
  const std::stringstream log() const;

  /// Returns signature member.
  const auto& signature() const { return m_signature; }

  /// Sets signature member.
  ctags_entry& signature(const std::string& v);

private:
  std::string m_access, m_class, m_kind, m_signature;
};
}; // namespace wex
