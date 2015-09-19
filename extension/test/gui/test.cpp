////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include <wx/extension/toolbar.h>
#include "test.h"

wxExStatusBar* fixture::m_StatusBar = NULL;
wxExManagedFrame* fixture::m_Frame = NULL;

fixture::fixture()
  : wxExTestFixture() 
  , m_Abbreviations{
      {"XX","GREAT"},
      {"YY","WHITE"},
      {"ZZ","SHARK"}}
  , m_BuiltinVariables{
      "Cb", "Cc", "Ce", "Cl", "Created", "Date", "Datetime",
      "Filename", "Fullpath", "Nl", "Path", "Time", "Year"}
{
  if (m_Frame == NULL)
  {
    m_Frame = new wxExManagedFrame(NULL, wxID_ANY, wxTheApp->GetAppDisplayName());
    m_Frame->Show();
    
    // Create the global lexers object, 
    // it should be present in ~/.wxex-test-gui
    // (depending on platform, configuration).
    wxExLexers::Get();
    
    wxConfigBase::Get()->Write(_("vi mode"), true);
    
    std::vector<wxExStatusBarPane> panes;

    panes.push_back(wxExStatusBarPane());
    
    for (int i = 0; i < 15; i++)
    {
      panes.push_back(wxExStatusBarPane(wxString::Format("Pane%d", i)));
    }
    
    panes.push_back(wxExStatusBarPane("PaneInfo"));
    panes.push_back(wxExStatusBarPane("PaneLexer"));
    panes.push_back(wxExStatusBarPane("PaneFileType"));
    panes.push_back(wxExStatusBarPane("LastPane"));

    m_StatusBar = m_Frame->SetupStatusBar(panes);
    
    CPPUNIT_ASSERT( m_StatusBar->GetFieldsCount () == panes.size());
  }
} 

void fixture::Process(const std::string& str, wxExSTCShell* shell)
{
  for (unsigned i = 0; i < str.length(); ++i)
  {
    shell->ProcessChar(str.at(i));
  }
}
