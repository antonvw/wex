////////////////////////////////////////////////////////////////////////////////
// Name:      data/layout.h
// Purpose:   Declaration of wex::data::layout class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

class wxFlexGridSizer;
class wxWindow;

namespace wex::data
{
/// Offers user data to be used by item layout.
class layout
{
public:
  typedef wxFlexGridSizer sizer_t;

  /// Constructor, specify parent and creates the sizer based on cols and rows.
  layout(wxWindow* parent, int cols, int rows = 0);

  /// Returns is_readonly.
  bool is_readonly() const { return m_is_readonly; };

  /// Sets is_readonly.
  layout& is_readonly(bool rhs);

  /// Returns the parent.
  wxWindow* parent() { return m_parent; };

  /// Returns the base sizer.
  auto* sizer() { return m_sizer; };

  /// Grows last row of sizer. Returns false if row cannot grow.
  bool sizer_grow_row();

  /// Returns the layout sizer.
  auto* sizer_layout() { return m_fgz; };

  /// Creates the sizer layout.
  bool sizer_layout_create(sizer_t* rhs);

  /// Grows last col of sizer layout. Returns false if col cannot grow.
  bool sizer_layout_grow_col();

  /// Grows last row of sizer layout. Returns false if row cannot grow.
  bool sizer_layout_grow_row();

private:
  bool sizer_can_grow_row() const;
  bool sizer_layout_can_grow_row() const;

  sizer_t * m_fgz{nullptr}, *m_sizer{nullptr};
  wxWindow* m_parent{nullptr};

  bool m_is_readonly{false};
};
} // namespace wex::data
