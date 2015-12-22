////////////////////////////////////////////////////////////////////////////////
// Name:      test-managedframe.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/vi.h>
#include "test.h"

// Also test the toolbar (wxExToolBar).
TEST_CASE("wxExManagedFrame", "[stc]")
{
  REQUIRE(GetFrame()->AllowClose(100, nullptr));
  
  wxExSTC* stc = new wxExSTC(GetFrame(), "hello world");
  AddPane(GetFrame(), stc);
  
  stc->SetFocus();
  stc->Show();
  wxExVi* vi = &stc->GetVi();
  
  wxExSTC* stco = nullptr;  
  REQUIRE(!GetFrame()->ExecExCommand(":n", stco));
  REQUIRE( stco == nullptr);
  
  GetFrame()->GetExCommand(vi, "/");
  
  GetFrame()->HideExBar(wxExManagedFrame::HIDE_BAR);
  GetFrame()->HideExBar(wxExManagedFrame::HIDE_BAR_FOCUS_STC);
  GetFrame()->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE);
  GetFrame()->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
  
  REQUIRE(!GetFrame()->GetManager().GetPane("VIBAR").IsShown());
  
  GetFrame()->GetFileHistory().Clear();
  
  wxMenu* menu = new wxMenu();
  GetFrame()->GetFileHistory().UseMenu(1000, menu);
  GetFrame()->SetFindFocus(GetFrame()->GetSTC());
  GetFrame()->OpenFile(GetTestFile());
  
  GetFrame()->SetRecentFile(GetTestFile().GetFullPath());
  GetFrame()->SetRecentFile("testing");
  
  REQUIRE( GetFrame()->GetFileHistory().GetHistoryFile().Contains("test.h"));
  REQUIRE( GetFrame()->GetFileHistory().GetCount() > 0);
  REQUIRE(!GetFrame()->GetFileHistory().GetVector(5).empty());
  
  GetFrame()->ShowExMessage("hello from GetFrame()");
  REQUIRE(!GetFrame()->ShowPane("xxxx"));
  REQUIRE(!GetFrame()->ShowPane("xxxx", false));
  GetFrame()->PrintEx(vi, "hello vi");
  
  GetFrame()->SyncAll();
  GetFrame()->SyncCloseAll(0);
  
  REQUIRE( GetFrame()->GetToolBar() != nullptr);
  REQUIRE( GetFrame()->GetOptionsToolBar() != nullptr);
  
  GetFrame()->GetToolBar()->AddControls();
  REQUIRE( GetFrame()->TogglePane("FINDBAR"));
  REQUIRE( GetFrame()->GetManager().GetPane("FINDBAR").IsShown());
  REQUIRE( GetFrame()->TogglePane("OPTIONSBAR"));
  REQUIRE( GetFrame()->GetManager().GetPane("OPTIONSBAR").IsShown());
  REQUIRE( GetFrame()->TogglePane("TOOLBAR"));
  REQUIRE(!GetFrame()->GetManager().GetPane("TOOLBAR").IsShown());
  REQUIRE( GetFrame()->ShowPane("TOOLBAR"));
  REQUIRE( GetFrame()->TogglePane("VIBAR"));
  REQUIRE( GetFrame()->GetManager().GetPane("VIBAR").IsShown());
  
  REQUIRE(!GetFrame()->TogglePane("XXXXBAR"));
  REQUIRE(!GetFrame()->GetManager().GetPane("XXXXBAR").IsOk());
  
  GetFrame()->OnNotebook(100, stc);
  
  GetFrame()->AppendPanes(menu);

  for (auto id : std::vector<int> {
    wxID_PREFERENCES, ID_FIND_FIRST, 
    ID_VIEW_LOWEST + 1, ID_VIEW_LOWEST + 2}) 
  {
    wxPostEvent(GetFrame(), wxCommandEvent(wxEVT_MENU, id));
  }
}
