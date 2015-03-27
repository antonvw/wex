////////////////////////////////////////////////////////////////////////////////
// Name:      test-filedlg.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/filedlg.h>
#include <wx/extension/file.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testFileDialog()
{
  wxExFile file;
  wxExFileDialog dlg(m_Frame, &file);
  
  CPPUNIT_ASSERT(dlg.ShowModalIfChanged(false) == wxID_OK);
}
