////////////////////////////////////////////////////////////////////////////////
// Name:      test-dirctrl.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/dir.h> // necessary?
#include <wx/extension/report/dirctrl.h>
#include "test.h"

void fixture::testDirCtrl()
{
  wxExGenericDirCtrl* ctrl = new wxExGenericDirCtrl(m_Frame, m_Frame);
  
  ctrl->ExpandAndSelectPath("./");
  
  for (auto id : std::vector<int> {
    ID_EDIT_VCS_LOWEST + 1, ID_TOOL_LOWEST + 1, 
    ID_TREE_COPY, ID_EDIT_OPEN, ID_TREE_RUN_MAKE,
    ID_TOOL_REPORT_FIND})
  {
    wxPostEvent(ctrl, wxCommandEvent(wxEVT_MENU, id));
  }
}
