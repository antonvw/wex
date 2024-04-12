////////////////////////////////////////////////////////////////////////////////
// Name:      data/layout.cpp
// Purpose:   Implementation of wex::data::layout
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/layout.h>
#include <wx/sizer.h>

// before using layout:
// When using a wxGridBagSizer instead of a wxFlexGridSizer:
// wxWidgets/src/common/gbsizer.cpp(781):
// assert ""Assert failure"" failed in Insert():
// Insert should not be used with wxGridBagSizer.
// during sizer->Add, and all item dialogs are incorrect
// when using layout:
// constructor for wxGridBagSizer is not ok
// see also wex::item::add_items and 'page cols', not yet working

wex::data::layout::layout(wxWindow* parent, int cols, int rows)
  : m_parent(parent)
  , m_sizer(rows > 0 ? new sizer_t(rows, cols, 0, 0) : new sizer_t(cols))
{
}

wex::data::layout& wex::data::layout::is_readonly(bool rhs)
{
  m_is_readonly = rhs;
  return *this;
}

bool wex::data::layout::sizer_can_grow_row() const
{
  return m_sizer->GetEffectiveRowsCount() >= 1 &&
         !m_sizer->IsRowGrowable(m_sizer->GetEffectiveRowsCount() - 1);
}

bool wex::data::layout::sizer_grow_row()
{
  if (!sizer_can_grow_row())
  {
    return false;
  }

  m_sizer->AddGrowableRow(m_sizer->GetEffectiveRowsCount() - 1);

  return true;
}

bool wex::data::layout::sizer_layout_can_grow_row() const
{
  return m_fgz != nullptr && m_fgz->GetEffectiveRowsCount() >= 1 &&
         !m_fgz->IsRowGrowable(m_fgz->GetEffectiveRowsCount() - 1);
}

bool wex::data::layout::sizer_layout_create(sizer_t* rhs)
{
  m_fgz = rhs;

  return rhs != nullptr;
}

bool wex::data::layout::sizer_layout_grow_col()
{
  if (
    m_fgz == nullptr || m_fgz->GetEffectiveColsCount() == 0 ||
    m_fgz->IsColGrowable(m_fgz->GetEffectiveColsCount() - 1))
  {
    return false;
  }

  m_fgz->AddGrowableCol(m_fgz->GetCols() - 1);

  return true;
}

bool wex::data::layout::sizer_layout_grow_row()
{
  if (!sizer_layout_can_grow_row())
  {
    return false;
  }

  m_fgz->AddGrowableRow(m_fgz->GetEffectiveRowsCount() - 1);

  return true;
}
