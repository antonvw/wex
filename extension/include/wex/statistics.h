////////////////////////////////////////////////////////////////////////////////
// Name:      statistics.h
// Purpose:   Include file for statistics classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <utility>
#include <vector>
#include <wex/grid.h>

namespace wex
{
  template <class T> class statistics;

  /// Helper class for adding clear menu to the grid, and 
  /// calling clear for the statistics.
  template <class T> class grid_statistics: public grid
  {
  public:
    /// Constructor.
    grid_statistics(
      statistics <T> * statistics,
      const window_data& data = window_data().style(wxWANTS_CHARS))
      : grid(data)
      , m_Statistics(statistics)
    {
      Bind(wxEVT_MENU, [=](wxCommandEvent& event) {m_Statistics->clear();}, wxID_CLEAR);
    }
  protected:
    void build_popup_menu(menu& menu) override {
      menu.style().reset();
      menu.style().set(menu::ALLOW_CLEAR);
      if (IsSelection()) menu.style().set(menu::IS_SELECTED);
      grid::build_popup_menu(menu);};
  private:
    statistics <T> * m_Statistics;
  };

  /// Offers base statistics. All statistics involve a key value pair,
  /// where the key is a string, and the value a template.
  /// The statistics can be shown on a grid, that is automatically
  /// updated whenever statistics change.
  template <class T> class statistics
  {
  public:
    /// Default constructor.
    /// You can specify a vector of values to initialize the statistics.
    statistics(const std::vector<std::pair<const std::string, T>> & v = {}) {
      for (const auto& it : v)
      {
        set(it.first, it.second);
      }};

    /// Adds other statistics.
    statistics& operator+=(const statistics& s) {
      for (const auto& it : s.m_Items)
      {
        inc(it.first, it.second);
      }
      return *this;}

    /// Clears the items. If you have shown the statistics
    /// the window is updated as well.
    void clear() {
      m_Items.clear();
      m_Rows.clear();

      if (m_Grid != nullptr)
      {
        m_Grid->ClearGrid();
        if (m_Grid->GetNumberRows() > 0)
        {
          m_Grid->DeleteRows(0, m_Grid->GetNumberRows());
        }
      }
    };

    /// Decrements key with value.
    const T dec(const std::string& key, T dec_value = 1) {
      return set(key, get(key) - dec_value);};

    /// Returns all items as a string. All items are returned as a string,
    /// with comma's separating items, and a : separating key and value.
    const std::string get() const {
      std::string text;
      for (const auto& it : m_Items)
      {
        if (!text.empty())
        {
          text += ", ";
        }
        
        text += it.first + ":" + std::to_string(it.second);
      }
      return text;};

    /// Returns value for specified key.
    const T get(const std::string& key) const {
      const auto it = m_Items.find(key);
      return it != m_Items.end() ? it->second: T();};

    /// Returns the items.
    const auto & get_items() const {return m_Items;};

    /// Increments key with value.
    const T inc(const std::string& key, T inc_value = 1) {
      return set(key, get(key) + inc_value);};

    /// Sets key to value. If you have shown the statistics
    /// the window is updated as well.
    const T set(const std::string& key, T value) {
      m_Items[key] = value;
      if (m_Grid != nullptr)
      {
        const auto& it = m_Rows.find(key);

        if (it != m_Rows.end())
        {
          m_Grid->SetCellValue(it->second, 1, std::to_string(value));
        }
        else
        {
          m_Grid->AppendRows(1);

          const auto row = m_Grid->GetNumberRows() - 1;
          m_Rows.insert({key, row});

          m_Grid->SetCellValue(row, 0, key);
          m_Grid->SetCellValue(row, 1, std::to_string(value));

          m_Grid->AutoSizeColumn(0);
        }

        m_Grid->ForceRefresh();
      }
      return value;};

    /// Shows the statistics as a grid window on the parent,
    /// and specify whether to show row labels and col labels.
    /// Returns the window that is created, or is activated,
    /// if it already was created.
    grid* show(
      /// the parent for the grid
      wxWindow* parent,
      /// show row labels (i.e. row numbers)
      bool hide_row_labels = true,
      /// show col labels (Item, Value)
      bool hide_col_labels = true,
      /// the id of the grid component
      wxWindowID id = wxID_ANY)
      {
      if (m_Grid == nullptr)
      {
        m_Grid = new grid_statistics<T>(this,
          window_data().style(wxWANTS_CHARS).id(id));
        m_Grid->CreateGrid(0, 0);
        m_Grid->AppendCols(2);
        m_Grid->EnableEditing(false);

#if wxUSE_DRAG_AND_DROP
        m_Grid->use_drag_and_drop(false);
#endif

        if (hide_row_labels)
        {
          m_Grid->HideRowLabels();
        }
        m_Grid->SetColLabelValue(0, _("Item"));
        m_Grid->SetColLabelValue(1, _("Value"));

        // Values are numbers.
        m_Grid->SetColFormatNumber(1);

        if (hide_col_labels)
        {
          m_Grid->HideColLabels();
        }

        for (const auto& it : m_Items)
        {
          m_Grid->AppendRows(1);
          const auto row = m_Grid->GetNumberRows() - 1;
          m_Grid->SetCellValue(row, 0, it.first);
          m_Grid->SetCellValue(row, 1, std::to_string(it.second));
          m_Rows[it.first] = row;
        }

        m_Grid->AutoSizeColumn(0);
      }
      m_Grid->Show();
      return m_Grid;}

    /// Access to the grid, returns nullptr if the grid has not been shown yet.
    const grid* get_grid() const {return m_Grid;}
  private:
    std::map<std::string, T> m_Items;
    std::map<std::string, int> m_Rows;
    grid_statistics<T>* m_Grid {nullptr};
  };
};