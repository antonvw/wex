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
#include <wex/toolbar.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::toolbar")
{
  frame()->get_toolbar()->add_controls(false);
  frame()->get_toolbar()->add_controls();
  
  frame()->get_toolbar()->add_tool(wxID_FIND);
  frame()->get_toolbar()->add_tool(wxID_CLEAR);
  frame()->get_toolbar()->add_tool(wxID_PREFERENCES);
  frame()->get_toolbar()->Realize();
  
  frame()->get_options_toolbar()->add_controls();
  
  frame()->manager().GetPane("FINDBAR").Show();
  frame()->manager().GetPane("OPTIONSBAR").Show();
  frame()->manager().Update();
  
  // Send events to the find toolbar.
  wxKeyEvent event(wxEVT_CHAR);
  
  for (auto key : std::vector<int> {WXK_UP, WXK_DOWN, WXK_HOME, WXK_END,
    WXK_PAGEUP, WXK_PAGEDOWN}) 
  {
    event.m_keyCode = key;
    wxPostEvent(frame()->get_find_toolbar(), event);
  }

  frame()->manager().GetPane("FINDBAR").Hide();
  frame()->manager().GetPane("OPTIONSBAR").Hide();
  frame()->manager().Update();
}
