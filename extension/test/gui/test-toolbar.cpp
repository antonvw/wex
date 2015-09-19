////////////////////////////////////////////////////////////////////////////////
// Name:      test-toolbar.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
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

void fixture::testToolBar()
{
  m_Frame->GetToolBar()->AddControls(false);
  m_Frame->GetToolBar()->AddControls();
  
  m_Frame->GetToolBar()->AddTool(wxID_FIND);
  m_Frame->GetToolBar()->AddTool(wxID_CLEAR);
  m_Frame->GetToolBar()->AddTool(wxID_PREFERENCES);
  m_Frame->GetToolBar()->Realize();
  
  m_Frame->GetManager().GetPane("FINDBAR").Show();
  m_Frame->GetManager().GetPane("OPTIONSBAR").Show();
  m_Frame->GetManager().Update();
}
