////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-entry.h
// Purpose:   Declaration of class wxExCTagsEntry
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

/// Offers information about a specific tag (see tagEntry).
class wxExCTagsEntry
{
public:
  /// Default constructor.
  wxExCTagsEntry() {;};

  /// Returns true if one of the members is filled.
  bool Active() const;

  /// Returns access member.
  const auto & Access() const {return m_access;};

  /// Sets access member.
  wxExCTagsEntry& Access(const std::string& v);

  /// Returns class member.
  const auto & Class() const {return m_class;};

  /// Sets class member.
  wxExCTagsEntry& Class(const std::string& v);

  /// Clear the member.
  void Clear();

  /// Returns member as a string.
  const std::string Get() const;

  /// Returns kind member.
  const auto & Kind() const {return m_kind;};

  /// Sets kind of tag. The value depends on the language.  For C and
  /// C++ these kinds are recommended:
  /// -c	class name
  /// -d	define (from #define XXX)
  /// -e	enumerator
  /// -f	function or method name
  /// -F	file name
  /// -g	enumeration name
  /// -m	member (of structure or class data)
  /// -p	function prototype
  /// -s	structure name
  /// -t	typedef
  /// -u	union name
  /// -v	variable
  wxExCTagsEntry& Kind(const std::string& v);

  /// Returns signature member.
  const auto & Signature() const {return m_signature;};

  /// Sets signature member.
  wxExCTagsEntry& Signature(const std::string& v);
private:
  std::string m_access, m_class, m_kind, m_signature;
};
