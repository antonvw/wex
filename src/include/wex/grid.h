////////////////////////////////////////////////////////////////////////////////
// Name:      grid.h
// Purpose:   Declaration of wex::grid class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/grid.h>
#include <wex/menu.h>
#include <wex/window-data.h>

namespace wex
{
  /// Offers popup menu with copy/paste, printing.
  /// It also offers drag/drop functionality.
  class grid : public wxGrid
  {
  public:
    /// Default constructor.
    grid(const window_data& data = window_data().style(wxWANTS_CHARS));

    /// Virtual Interface.
    /// This one is invoked after IsAllowedDropSelection, and drops the data.
    /// Default it calls set_cells_value.
    /// If you return true, the selection is correctly dropped,
    /// and the (old) selection is emptied and cleared to simulate dragging.
    virtual bool drop_selection(
      const wxGridCellCoords& drop_coords, const std::string& data);

    /// This one is invoked before dragging, and you can indicate
    /// whether you like the current selection to be dragged elsewhere.
    /// Default it is allowed.
    virtual bool is_allowed_drag_selection();

    /// This is invoked after dragging and before anything is really dropped.
    /// Default it checks for each cell whether it is read-only, etc.
    virtual bool is_allowed_drop_selection(
      const wxGridCellCoords& drop_coords, const std::string& data);

    /// This one is called by empty_selection, set_cells_value,
    /// and so during drag/drop as well, and allows you to
    /// override default here (which simply calls SetCellValue).
    /// So it is on a cell basis, whereas the DropSelection is on a range basis.
    virtual void set_cell_value(
      const wxGridCellCoords& coords, const std::string& data);
    
    /// Other methods

    /// Copy from selected cells.
    bool copy_selected_cells_to_clipboard() const;

    /// Empties selected cells.
    void empty_selection();

    /// Finds next.
    bool find_next(const std::string& text, bool forward = true);

    /// Updates find replace text.
    const std::string get_find_string() const;

    /// Get text from selected cells,
    const std::string get_selected_cells_value() const;

    /// Paste starting at current grid cursor.
    void paste_cells_from_clipboard();

    /// Prints the grid.
    void print();

    /// Previews the grid.
    void print_preview();

    /// Fill cells with text starting at a cel.
    void set_cells_value(const wxGridCellCoords& start_coords, const std::string& data);

    /// Specify whether you want to use drag/drop.
    /// Default it is used.
    void use_drag_and_drop(bool use);
  protected:
    /// Builds the page used for printing.
    const std::string build_page();

    /// Builds the popup menu.
    virtual void build_popup_menu(menu& menu);
  private:
    bool m_use_drag_and_drop {true};
  };
};
