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
#include "test.h"

void fixture::testToolBar()
{
  wxExToolBar* tb = new wxExToolBar(m_Frame);
  wxExFindToolBar* fb = new wxExFindToolBar(m_Frame);
  wxExOptionsToolBar* ob = new wxExOptionsToolBar(m_Frame);
  
  tb->AddTool(wxID_FIND);
  tb->AddTool(wxID_CLEAR);
  tb->AddTool(wxID_PREFERENCES);
  
  tb->AddControls();
  
  CPPUNIT_ASSERT(tb->GetFrame() == m_Frame);
  CPPUNIT_ASSERT(fb->GetFrame() == m_Frame);
  CPPUNIT_ASSERT(ob->GetFrame() == m_Frame);
}
