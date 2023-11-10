////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-entry.h
// Purpose:   Declaration of class wex::ctags_entry
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include <readtags.h>
#include <wex/core/reflection.h>

class wxStyledTextCtrl;

namespace wex
{

/// Offers information about a specific tag (see tagEntry).
class ctags_entry
{
public:
  /// Static interface.

  /// Register image on stc component.
  static void register_image(wxStyledTextCtrl* stc);

  /// Other methods.

  /// Default constructor.
  ctags_entry();

  /// Returns access member.
  const auto& access() const { return m_access; }

  /// Sets access member.
  ctags_entry& access(const std::string& v);

  /// Returns class member.
  const auto& class_name() const { return m_class; }

  /// Sets class member.
  ctags_entry& class_name(const std::string& v);

  /// Clears the members.
  void clear();

  /// Returns entry.
  auto& entry() { return m_entry; }

  /// Returns const entry.
  const auto& entry() const { return m_entry; }

  /// Returns complete entry as a string.
  /// Contains image and name, and might assign
  /// the signature.
  const std::string entry_string(size_t min_size) const;

  /// Returns filter name.
  const std::string filter() const;

  /// Sets filter.
  ctags_entry& filter(const ctags_entry& entry);

  /// Returns true if one of the members is filled.
  bool is_active() const;

  /// C++ these kinds are recommended:
  /// - c	class name
  bool is_class_name() const { return m_kind == "c"; }

  /// - d	define (from define XXX)
  bool is_define() const { return m_kind == "d"; }

  /// - e	enumerator
  bool is_enumerator() const { return m_kind == "e"; }

  /// - g	enumeration name
  bool is_enumeration_name() const { return m_kind == "g"; }

  /// - F	file name
  bool is_file_name() const { return m_kind == "F"; }

  /// - f	function or method name
  bool is_function() const { return m_kind == "f"; }

  /// Combination.
  bool is_function_or_prototype() const
  {
    return is_function() || is_function_prototype();
  }

  /// - p	function prototype
  bool is_function_prototype() const { return m_kind == "p"; }

  /// Returns true if this is a master entry.
  bool is_master() const;

  /// - m	member (of structure or class data)
  bool is_member() const { return m_kind == "m"; }

  /// - s	structure name
  bool is_structure_name() const { return m_kind == "s"; }

  /// - t	typedef
  bool is_typedef() const { return m_kind == "t"; }

  /// - u	union name
  bool is_union_name() const { return m_kind == "u"; }

  /// - v	variable
  bool is_variable() const { return m_kind == "v"; }

  /// Returns kind.
  auto& kind() const { return m_kind; }

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
  bool entry_equal(const std::string& text, const std::string& field) const;
  std::string image_string() const;
  std::string signature_and_image() const;

  tagEntry m_entry{0};

  std::string m_access, m_class, m_kind, m_signature;

  reflection m_reflect;
};
}; // namespace wex
