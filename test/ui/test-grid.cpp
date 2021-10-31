////////////////////////////////////////////////////////////////////////////////
// Name:      test-grid.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/grid.h>

#include "test.h"

TEST_CASE("wex::grid")
{
  auto* grid = new wex::grid();
  frame()->pane_add(grid);

  REQUIRE(grid->CreateGrid(5, 5));

  grid->set_cell_value(wxGridCellCoords(0, 0), "test");

  grid->GoToCell(0, 0);
  REQUIRE(grid->get_selected_cells_value().empty());
  REQUIRE(grid->GetCellValue(0, 0) == "test");

  grid->set_cells_value(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  REQUIRE(grid->GetCellValue(0, 0) == "test1");

  grid->ClearSelection();
  grid->empty_selection();
  grid->SetFocus();

  REQUIRE(grid->find_next("test1"));
  REQUIRE(grid->get_find_string() == "test1");
  REQUIRE(!grid->find_next("text1"));

  grid->paste_cells_from_clipboard();
  grid->use_drag_and_drop(true);
  grid->use_drag_and_drop(false);
}
