////////////////////////////////////////////////////////////////////////////////
// Name:      grid-statistics.h
// Purpose:   Include file for class wex::grid_statistics
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/grid.h>
#include <wex/statistics.h>

namespace wex
{
  /// Helper class for adding clear menu to the grid, and
  /// calling clear for the statistics.
  template <class T>
  class grid_statistics
    : public grid
    , public statistics<T>
  {
  public:
    /// Constructor.
    grid_statistics(
      const data::window& data = data::window().style(wxWANTS_CHARS))
      : grid(data)
    {
      Bind(
        wxEVT_MENU,
        [&, this](wxCommandEvent& event) {
          clear();
        },
        wxID_CLEAR);

      CreateGrid(0, 0);
      AppendCols(2);
      EnableEditing(false);
      use_drag_and_drop(false);
    };

    void clear() override
    {
      statistics<T>::clear();
      m_rows.clear();

      ClearGrid();
      if (GetNumberRows() > 0)
      {
        DeleteRows(0, GetNumberRows());
      };
    };

    const T set(const std::string& key, T value) override
    {
      statistics<T>::set(key, value);

      const auto& it = m_rows.find(key);

      if (it != m_rows.end())
      {
        SetCellValue(it->second, 1, std::to_string(value));
      }
      else
      {
        AppendRows(1);

        const auto row = GetNumberRows() - 1;
        m_rows.insert({key, row});

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
        m_rows[it.first] = row;
      }

      AutoSizeColumn(0);
      Show();
      return (grid*)this;
    };

  protected:
    void build_popup_menu(menu& menu) override
    {
      menu.style().reset();
      menu.style().set(menu::ALLOW_CLEAR);
      if (IsSelection())
        menu.style().set(menu::IS_SELECTED);
      grid::build_popup_menu(menu);
    };

  private:
    std::map<std::string, int> m_rows;
  };
}; // namespace wex
