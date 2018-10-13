////////////////////////////////////////////////////////////////////////////////
// Name:      test-toolbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/toolbar.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wex::toolbar")
{
  GetFrame()->GetToolBar()->AddControls(false);
  GetFrame()->GetToolBar()->AddControls();
  
  GetFrame()->GetToolBar()->AddTool(wxID_FIND);
  GetFrame()->GetToolBar()->AddTool(wxID_CLEAR);
  GetFrame()->GetToolBar()->AddTool(wxID_PREFERENCES);
  GetFrame()->GetToolBar()->Realize();
  
  GetFrame()->GetOptionsToolBar()->AddControls();
  
  GetFrame()->GetManager().GetPane("FINDBAR").Show();
  GetFrame()->GetManager().GetPane("OPTIONSBAR").Show();
  GetFrame()->GetManager().Update();
  
  // Send events to the find toolbar.
  wxKeyEvent event(wxEVT_CHAR);
  
  for (auto key : std::vector<int> {WXK_UP, WXK_DOWN, WXK_HOME, WXK_END,
    WXK_PAGEUP, WXK_PAGEDOWN}) 
  {
    event.m_keyCode = key;
    wxPostEvent(GetFrame()->GetFindToolBar(), event);
  }

  GetFrame()->GetManager().GetPane("FINDBAR").Hide();
  GetFrame()->GetManager().GetPane("OPTIONSBAR").Hide();
  GetFrame()->GetManager().Update();
}
