////////////////////////////////////////////////////////////////////////////////
// Name:      test-managed_frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/managedframe.h>
#include <wex/defs.h>
#include <wex/stc.h>
#include <wex/toolbar.h>
#include <wex/vi.h>
#include "test.h"

// Also test the toolbar (wex::toolbar).
TEST_CASE("wex::managed_frame")
{
  REQUIRE(GetFrame()->AllowClose(100, nullptr));
  
  GetSTC()->SetFocus();
  GetSTC()->Show();
  wex::vi* vi = &GetSTC()->GetVi();
  
  wex::ex_command command(":n");
  REQUIRE(!GetFrame()->ExecExCommand(command));
  
  REQUIRE(!GetFrame()->GetExCommand(vi, ""));
  REQUIRE(!GetFrame()->GetExCommand(vi, "x"));
  REQUIRE(!GetFrame()->GetExCommand(vi, "xx"));
  REQUIRE( GetFrame()->GetExCommand(vi, "/"));
  REQUIRE( GetFrame()->GetExCommand(vi, "?"));
  REQUIRE( GetFrame()->GetExCommand(vi, "="));
  
  REQUIRE(!GetFrame()->SaveCurrentPage("key"));
  REQUIRE( GetFrame()->RestorePage("key") == nullptr);
  
  GetFrame()->HideExBar(wex::managed_frame::HIDE_BAR);
  GetFrame()->HideExBar(wex::managed_frame::HIDE_BAR_FOCUS_STC);
  GetFrame()->HideExBar(wex::managed_frame::HIDE_BAR_FORCE);
  GetFrame()->HideExBar(wex::managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
  
  REQUIRE(!GetFrame()->GetManager().GetPane("VIBAR").IsShown());
  
  GetFrame()->GetFileHistory().Clear();
  
  wxMenu* menu = new wxMenu();
  GetFrame()->GetFileHistory().UseMenu(1000, menu);
  GetFrame()->SetFindFocus(GetFrame()->GetSTC());
  GetFrame()->OpenFile(GetTestPath("test.h"));
  
  GetFrame()->SetRecentFile(GetTestPath("test.h"));
  GetFrame()->SetRecentFile("testing");
  
  REQUIRE( GetFrame()->GetFileHistory().GetHistoryFile().Path().string().find("test.h") != std::string::npos);
  REQUIRE( GetFrame()->GetFileHistory().GetCount() > 0);
  REQUIRE(!GetFrame()->GetFileHistory().GetHistoryFiles(5).empty());
  
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
  REQUIRE(!GetFrame()->GetManager().GetPane("XXXXBAR").is_ok());
  
  GetFrame()->OnNotebook(100, GetSTC());
  
  GetFrame()->AppendPanes(menu);

#ifndef __WXMSW__
  for (auto id : std::vector<int> {
    wxID_PREFERENCES, 
    wex::ID_FIND_FIRST, wex::ID_FIND_LAST,
    wex::ID_CLEAR_FILES, wex::ID_CLEAR_FINDS,
    wex::ID_VIEW_LOWEST + 1, wex::ID_VIEW_LOWEST + 2, wex::ID_VIEW_LOWEST + 3, wex::ID_VIEW_LOWEST + 4}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(GetFrame(), event);
  }
#endif
}
