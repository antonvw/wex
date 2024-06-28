////////////////////////////////////////////////////////////////////////////////
// Name:      grid-statistics.h
// Purpose:   Include file for class wex::grid_statistics
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/common/statistics.h>
#include <wex/ui/grid.h>

namespace wex
{
/// Offers a class to show statistics on a grid.
template <class T>
class grid_statistics
  : public grid
  , public statistics<T>
{
public:
  /// Constructor.
  grid_statistics(
    const std::vector<std::pair<const std::string, T>>& v = {},
    const data::window& data = data::window().style(wxWANTS_CHARS))
    : statistics<T>(v)
    , grid(data)
  {
    Bind(
      wxEVT_MENU,
      [&, this](wxCommandEvent& event)
      {
        clear();
      },
      wxID_CLEAR);

    CreateGrid(0, 0);
    AppendCols(2);
    EnableEditing(false);
    use_drag_and_drop(false);
  };

  /// Clears statistics and grid.
  void clear() override
  {
    statistics<T>::clear();
    m_keys.clear();

    ClearGrid();

    if (GetNumberRows() > 0)
    {
      DeleteRows(0, GetNumberRows());
    };
  };

  /// Returns keys.
  const auto& get_keys() const { return m_keys; }

  /// Sets key to value, and refreshes grid.
  const T set(const std::string& key, T value) override
  {
    statistics<T>::set(key, value);

    if (const auto& it = m_keys.find(key); it != m_keys.end())
    {
      SetCellValue(it->second, 1, std::to_string(value));
    }
    else
    {
      AppendRows(1);

      const auto row = GetNumberRows() - 1;
      m_keys.insert({key, row});

      SetCellValue(row, 0, key);
      SetCellValue(row, 1, std::to_string(value));

      AutoSizeColumn(0);
    }

    ForceRefresh();

    return value;
  };

  /// Shows the statistics as a grid window on the parent,
  /// and specify whether to show row labels and col labels.
  /// Returns the window that is created, or is activated,
  /// if it already was created.
  grid* show(
    /// show row labels (i.e. row numbers)
    bool hide_row_labels = true,
    /// show col labels (Item, Value)
    bool hide_col_labels = true)
  {
    if (GetNumberRows() > 0)
    {
      DeleteRows(0, GetNumberRows());
    }

    if (hide_row_labels)
    {
      HideRowLabels();
    }
    SetColLabelValue(0, _("Item"));
    SetColLabelValue(1, _("Value"));

    // Values are numbers.
    SetColFormatNumber(1);

    if (hide_col_labels)
    {
      HideColLabels();
    }

    for (const auto& it : statistics<T>::get_items())
    {
      AppendRows(1);
      const auto row = GetNumberRows() - 1;
      SetCellValue(row, 0, it.first);
      SetCellValue(row, 1, std::to_string(it.second));
      m_keys[it.first] = row;
    }

    AutoSizeColumn(0);
    Show(true);
    return reinterpret_cast<grid*>(this);
  };

protected:
  void build_popup_menu(menu& menu) override
  {
    menu.style().reset();
    menu.style().set(menu::ALLOW_CLEAR);
    if (IsSelection())
    {
      menu.style().set(menu::IS_SELECTED);
    }
    grid::build_popup_menu(menu);
  };

private:
  std::map<std::string, int> m_keys;
};
}; // namespace wex
