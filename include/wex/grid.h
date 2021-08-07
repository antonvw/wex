////////////////////////////////////////////////////////////////////////////////
// Name:      grid.h
// Purpose:   Declaration of wex::grid class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/window.h>
#include <wex/factory/grid.h>
#include <wex/menu.h>

namespace wex
{
/// Offers popup menu with copy/paste, printing.
/// It also offers drag/drop functionality.
class grid : public factory::grid
{
public:
  /// Default constructor.
  explicit grid(const data::window& data = data::window().style(wxWANTS_CHARS));

  /// Copy from selected cells.
  bool copy_selected_cells_to_clipboard() const;

  /// This one is invoked after is_allowed_drop_selection, and drops the data.
  bool
  drop_selection(const wxGridCellCoords& drop_coords, const std::string& data);

  /// Empties selected cells.
  void empty_selection();

  /// Finds next.
  bool find_next(const std::string& text, bool forward = true);

  /// Get text from all cells.
  const std::string get_cells_value() const;

  /// Updates find replace text.
  const std::string get_find_string() const;

  /// Get text from selected cells.
  const std::string get_selected_cells_value() const;

  /// This is invoked after dragging and before anything is really dropped.
  /// It checks for each cell whether it is read-only, etc.
  bool is_allowed_drop_selection(
    const wxGridCellCoords& drop_coords,
    const std::string&      data);

  /// Paste starting at current grid cursor.
  void paste_cells_from_clipboard();

  /// Prints the grid.
  void print();

  /// Previews the grid.
  void print_preview();

  /// This one is called by empty_selection, set_cells_value,
  /// and so during drag/drop as well.
  /// So it is on a cell basis, whereas the drop_selection is on a range
  /// basis.
  void set_cell_value(const wxGridCellCoords& coords, const std::string& data);

  /// Fill cells with text starting at a cel.
  void set_cells_value(
    const wxGridCellCoords& start_coords,
    const std::string&      data);

  /// Specify whether you want to use drag/drop.
  /// Default it is used.
  void use_drag_and_drop(bool use);

protected:
  /// Virtual Interface.

  /// Builds the popup menu.
  virtual void build_popup_menu(menu& menu);

private:
  const std::string build_page();

  bool m_use_drag_and_drop{true};
};
}; // namespace wex
