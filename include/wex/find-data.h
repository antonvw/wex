////////////////////////////////////////////////////////////////////////////////
// Name:      find-data.h
// Purpose:   Declaration of class wex::data::find
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/stc/stc.h>

namespace wex::factory
{
  class stc;
}

namespace wex::data
{
  /// Offers a class to find data inside a core stc component.
  class find
  {
  public:
    /// Returns if recursive is true.
    static bool recursive() { return m_recursive; };

    /// Sets recursive.
    static void recursive(bool rhs) { m_recursive = rhs; };

    /// Constructor. Sets positions.
    find(
      /// component
      wex::factory::stc* stc,
      /// text to find
      const std::string& text,
      /// forward
      bool forward = true);

    /// Returns end pos.
    int end_pos() const { return m_end_pos; };

    /// Returns true if text found in margin, and sets line.
    bool find_margin(int& line);

    /// Returns find flags.
    int flags() const { return m_flags; };

    /// Sets find flags.
    find& flags(int rhs);

    /// Returns if forward search is true.
    bool is_forward() const { return m_forward; };

    /// Returns start pos.
    int start_pos() const { return m_start_pos; };

    /// Returns stc member.
    factory::stc* stc() { return m_stc; };

    /// Returns text.
    const auto& text() { return m_text; };

  private:
    void set_pos();

    factory::stc* m_stc{nullptr};

    int m_end_pos{wxSTC_INVALID_POSITION}, m_flags{-1},
      m_start_pos{wxSTC_INVALID_POSITION};

    const bool        m_forward;
    const std::string m_text;

    static inline bool m_recursive{false};
  };
}; // namespace wex::data
