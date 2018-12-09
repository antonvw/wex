////////////////////////////////////////////////////////////////////////////////
// Name:      test-grid.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/grid.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::grid")
{
  wex::grid* grid = new wex::grid();
  add_pane(frame(), grid);
  
  REQUIRE(grid->CreateGrid(5, 5));
  
  grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  
  grid->GoToCell(0, 0);
  REQUIRE( grid->get_selected_cells_value().empty());
  REQUIRE( grid->GetCellValue(0, 0) == "test");
  
  grid->set_cells_value(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  REQUIRE(grid->GetCellValue(0, 0) == "test1");
  
  grid->ClearSelection();
  grid->empty_selection();
  grid->SetFocus();
  
  REQUIRE( grid->find_next("test1"));
  REQUIRE( grid->get_find_string() == "test1");
  // REQUIRE( grid->get_selected_cells_value() == "test1");
  REQUIRE(!grid->find_next("text1"));
  
  REQUIRE(grid->copy_selected_cells_to_clipboard());
  
  grid->paste_cells_from_clipboard();
  
//  grid->Print();
  grid->print_preview();
  
#if wxUSE_DRAG_AND_DROP
  grid->use_drag_and_drop(true);
  REQUIRE(grid->IsAllowedDragSelection());
  grid->use_drag_and_drop(false);
#endif
}
