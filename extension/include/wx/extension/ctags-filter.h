////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-filter.h
// Purpose:   Declaration of class wxExCTagsFilter
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

/// Offers filters for wxExCTags.
class wxExCTagsFilter
{
public:
  /// Default constructor.
  wxExCTagsFilter() {;};

  /// Returns true if this filter is active.
  bool Active() const;

  /// Returns access filter.
  const auto & Access() const {return m_access;};

  /// Sets access filter.
  wxExCTagsFilter& Access(const std::string& v);

  /// Returns class filter.
  const auto & Class() const {return m_class;};

  /// Sets class filter.
  wxExCTagsFilter& Class(const std::string& v);

  /// Clear the filter.
  void Clear();

  /// Returns filter as a string.
  const std::string Get() const;

  /// Returns kind filter.
  const auto & Kind() const {return m_kind;};

  /// Sets kind filter.
  wxExCTagsFilter& Kind(const std::string& v);

  /// Returns signature filter.
  const auto & Signature() const {return m_signature;};

  /// Sets signature filter.
  wxExCTagsFilter& Signature(const std::string& v);
private:
  std::string m_access, m_class, m_kind, m_signature;
};
