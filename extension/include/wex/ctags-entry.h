////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-entry.h
// Purpose:   Declaration of class wex::ctags_entry
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
  /// Offers information about a specific tag (see tagEntry).
  class ctags_entry
  {
  public:
    /// Default constructor.
    ctags_entry() {;};

    /// Returns true if one of the members is filled.
    bool Active() const;

    /// Returns access member.
    const auto & Access() const {return m_access;};

    /// Sets access member.
    ctags_entry& Access(const std::string& v);

    /// Returns class member.
    const auto & Class() const {return m_class;};

    /// Sets class member.
    ctags_entry& Class(const std::string& v);

    /// Clear the member.
    void Clear();

    /// Returns member as a string.
    const std::string Get() const;

    /// Returns kind member.
    const auto & Kind() const {return m_kind;};

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
    ctags_entry& Kind(const std::string& v);

    /// Returns signature member.
    const auto & Signature() const {return m_signature;};

    /// Sets signature member.
    ctags_entry& Signature(const std::string& v);
  private:
    std::string m_access, m_class, m_kind, m_signature;
  };
};
