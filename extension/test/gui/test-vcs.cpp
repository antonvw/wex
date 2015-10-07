////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/menu.h>
#include <wx/extension/vcs.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testVCS()
{
  CPPUNIT_ASSERT(wxExVCS::GetCount() > 0);
  
  wxFileName file(GetTestFile());
  file.Normalize();
  
  // In wxExApp the vcs is Read, so current vcs is known,
  // using this constructor results in command id 0,
  // giving the first command of current vcs, being add.
  wxExVCS vcs(std::vector< wxString >{file.GetFullPath()});
  
  vcs.ConfigDialog(m_Frame, "test vcs", false);
  
  // DirExists
  CPPUNIT_ASSERT( vcs.DirExists(file));
  
  // Execute
  CPPUNIT_ASSERT( vcs.Execute());

  // GetCount
  CPPUNIT_ASSERT( vcs.GetCount() > 0);

  // GetEntry  
  CPPUNIT_ASSERT( vcs.GetEntry().BuildMenu(100, new wxMenu("test")) > 0);
  CPPUNIT_ASSERT( vcs.GetEntry().GetOutput().empty());
  CPPUNIT_ASSERT( vcs.GetEntry().GetCommand().GetCommand() == "add");
  
  // GetFileName
  CPPUNIT_ASSERT( vcs.GetFileName().IsOk());
  
  // GetName
  CPPUNIT_ASSERT( vcs.GetName() == "Auto");
  CPPUNIT_ASSERT(!vcs.GetEntry().GetCommand().IsOpen());
  
  // LoadDocument
  CPPUNIT_ASSERT( wxExVCS::LoadDocument());
  
  // SetEntryFromBase
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  CPPUNIT_ASSERT( vcs.SetEntryFromBase());
  
  // Use
  CPPUNIT_ASSERT( vcs.Use());
}
