////////////////////////////////////////////////////////////////////////////////
// Name:      statistics.h
// Purpose:   Include file for statistics classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <wx/extension/grid.h>

#if wxUSE_GRID
template <class T> class wxExStatistics;

/// Helper class for adding clear menu to the grid, and 
/// calling Clear for the statistics.
template <class T> class WXDLLIMPEXP_BASE wxExGridStatistics: public wxExGrid
{
public:
  /// Constructor.
  wxExGridStatistics(wxWindow* parent,
    wxExStatistics <T> * statistics,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxWANTS_CHARS,
    const wxString& name = wxGridNameStr)
    : wxExGrid(parent, id, pos, size, style, name)
    , m_Statistics(statistics)
  {
    Bind(wxEVT_MENU, [=](wxCommandEvent& event) {m_Statistics->Clear();}, wxID_CLEAR);
  }
protected:
  void BuildPopupMenu(wxExMenu& menu) {
    int style = wxExMenu::MENU_ALLOW_CLEAR;
    if (IsSelection()) style |= wxExMenu::MENU_IS_SELECTED;
    menu.SetStyle(style);
    wxExGrid::BuildPopupMenu(menu);};
private:
  wxExStatistics <T> * m_Statistics;
};
#endif

/// Offers base statistics. All statistics involve a key value pair,
/// where the key is a wxString, and the value a template.
/// The statistics can be shown on a grid, that is automatically
/// updated whenever statistics change.
template <class T> class WXDLLIMPEXP_BASE wxExStatistics
{
public:
  /// Default constructor.
  wxExStatistics() 
#if wxUSE_GRID
    : m_Grid(nullptr)
#endif
    {};

  /// Adds other statistics.
  wxExStatistics& operator+=(const wxExStatistics& s) {
    for (const auto& it : s.m_Items)
    {
      Inc(it.first, it.second);
    }
    return *this;}

  /// Clears the items. If you have Shown the statistics
  /// the window is updated as well.
  void Clear() {
    m_Items.clear();
#if wxUSE_GRID
    m_Rows.clear();

    if (m_Grid != nullptr)
    {
      m_Grid->ClearGrid();
      if (m_Grid->GetNumberRows() > 0)
      {
        m_Grid->DeleteRows(0, m_Grid->GetNumberRows());
      }
    }
#endif
  };

  /// Returns all items as a string. All items are returned as a string,
  /// with comma's separating items, and a : separating key and value.
  const wxString Get() const {
    wxString text;
    for (const auto& it : m_Items)
    {
      if (!text.empty())
      {
        text << ", ";
      }
      
      text << it.first << ":" << it.second;
    }
    return text;};

  /// Returns the items.
  const auto & GetItems() const {return m_Items;};

  /// Returns value for specified key.
  const T Get(const wxString& key) const {
    const auto it = m_Items.find(key);
    return it != m_Items.end() ? it->second: T();};

  /// Decrements key with value.
  const T Dec(const wxString& key, T dec_value = 1) {
    return Set(key, Get(key) - dec_value);};

  /// Increments key with value.
  const T Inc(const wxString& key, T inc_value = 1) {
    return Set(key, Get(key) + inc_value);};

  /// Sets key to value. If you have Shown the statistics
  /// the window is updated as well.
  const T Set(const wxString& key, T value) {
    m_Items[key] = value;
#if wxUSE_GRID
    if (m_Grid != nullptr)
    {
      const auto it = m_Rows.find(key);
      if (it != m_Rows.end())
      {
        m_Grid->SetCellValue(it->second, 1, std::to_string(value));
      }
      else
      {
        m_Grid->AppendRows(1);

        const int row = m_Grid->GetNumberRows() - 1;
        m_Rows.insert({key, row});

        m_Grid->SetCellValue(row, 0, key);
        m_Grid->SetCellValue(row, 1, std::to_string(value));

        m_Grid->AutoSizeColumn(0);
      }

      m_Grid->ForceRefresh();
    }
#endif
    return value;};

#if wxUSE_GRID
  /// Shows the statistics as a grid window on the parent,
  /// and specify whether to show row labels and col labels.
  /// Returns the window that is created, or is activated,
  /// if it already was created.
  wxExGrid* Show(
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
      m_Grid = new wxExGridStatistics<T>(parent, this, id);
      m_Grid->CreateGrid(0, 0);
      m_Grid->AppendCols(2);
      m_Grid->EnableEditing(false);

#if wxUSE_DRAG_AND_DROP
      m_Grid->UseDragAndDrop(false);
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
        const int row = m_Grid->GetNumberRows() - 1;
        m_Grid->SetCellValue(row, 0, it.first);
        m_Grid->SetCellValue(row, 1, std::to_string(it.second));
        m_Rows[it.first] = row;
      }
    }
    m_Grid->Show();
    return m_Grid;}

  /// Access to the grid, returns nullptr if the grid has not been shown yet.
  const wxExGrid* GetGrid() const {return m_Grid;}
#endif
private:
  std::map<wxString, T> m_Items;

#if wxUSE_GRID
  std::map<wxString, int> m_Rows;
  wxExGridStatistics<T>* m_Grid;
#endif
};
