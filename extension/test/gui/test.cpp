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
#include "test.h"

wxExStatusBar* wxExGuiTestFixture::m_StatusBar = NULL;

wxExGuiTestFixture::wxExGuiTestFixture()
  : wxExTestFixture() 
  , m_Frame((wxExManagedFrame *)wxTheApp->GetTopWindow())
  , m_Abbreviations{
      {"XX","GREAT"},
      {"YY","WHITE"},
      {"ZZ","SHARK"}}
  , m_BuiltinVariables{
      "Cb", "Cc", "Ce", "Cl", "Created", "Date", "Datetime",
      "Filename", "Fullpath", "Nl", "Path", "Time", "Year"}
{
  // Create the global lexers object, 
  // it should be present in ~/.wxex-test-gui
  // (depending on platform, configuration).
  wxExLexers::Get();
  
  wxConfigBase::Get()->Write(_("vi mode"), true);
} 

void wxExGuiTestFixture::Process(const std::string& str, wxExSTCShell* shell)
{
  for (unsigned i = 0; i < str.length(); ++i)
  {
    shell->ProcessChar(str.at(i));
  }
}
