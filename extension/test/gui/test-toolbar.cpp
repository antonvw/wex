////////////////////////////////////////////////////////////////////////////////
// Name:      test-toolbar.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/toolbar.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExToolBar")
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
  
  GetFrame()->GetManager().GetPane("FINDBAR").Hide();
  GetFrame()->GetManager().GetPane("OPTIONSBAR").Hide();
  GetFrame()->GetManager().Update();
}
