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
  AddPane(GetFrame(), grid);
  
  REQUIRE(grid->CreateGrid(5, 5));
  
  grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  
  grid->GoToCell(0, 0);
  REQUIRE( grid->GetSelectedCellsValue().empty());
  REQUIRE( grid->GetCellValue(0, 0) == "test");
  
  grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  REQUIRE(grid->GetCellValue(0, 0) == "test1");
  
  grid->ClearSelection();
  grid->EmptySelection();
  grid->SetFocus();
  
  REQUIRE( grid->FindNext("test1"));
  REQUIRE( grid->GetFindString() == "test1");
  // REQUIRE( grid->GetSelectedCellsValue() == "test1");
  REQUIRE(!grid->FindNext("text1"));
  
  REQUIRE(grid->CopySelectedCellsToClipboard());
  
  grid->PasteCellsFromClipboard();
  
//  grid->Print();
  grid->PrintPreview();
  
#if wxUSE_DRAG_AND_DROP
  grid->UseDragAndDrop(true);
  REQUIRE(grid->IsAllowedDragSelection());
  grid->UseDragAndDrop(false);
#endif
}
