////////////////////////////////////////////////////////////////////////////////
// Name:      test-grid.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/grid.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void wxExGuiTestFixture::testGrid()
{
  wxExGrid* grid = new wxExGrid(m_Frame);
  
  CPPUNIT_ASSERT(grid->CreateGrid(5, 5));
  
  grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  
  grid->GoToCell(0, 0);
  CPPUNIT_ASSERT( grid->GetSelectedCellsValue().empty());
  CPPUNIT_ASSERT( grid->GetCellValue(0, 0) == "test");
  
  grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  CPPUNIT_ASSERT(grid->GetCellValue(0, 0) == "test1");
  
  grid->ClearSelection();
  grid->EmptySelection();
  grid->SetFocus();
  
  CPPUNIT_ASSERT( grid->FindNext("test1"));
  CPPUNIT_ASSERT( grid->GetFindString() == "test1");
  // CPPUNIT_ASSERT( grid->GetSelectedCellsValue() == "test1");
  CPPUNIT_ASSERT(!grid->FindNext("text1"));
  
  CPPUNIT_ASSERT(grid->CopySelectedCellsToClipboard());
  
  grid->PasteCellsFromClipboard();
  
//  grid->Print();
  grid->PrintPreview();
  
#if wxUSE_DRAG_AND_DROP
  grid->UseDragAndDrop(true);
  CPPUNIT_ASSERT(grid->IsAllowedDragSelection());
  grid->UseDragAndDrop(false);
#endif
}
