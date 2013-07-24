////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class App
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/cmdline.h> // for wxCmdLineParser
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(App);

#ifdef __WXOSX__  
void App::MacOpenFiles(const wxArrayString& fileNames)
{
  Frame* frame = wxDynamicCast(GetTopWindow(), Frame);
  wxExOpenFiles(frame, fileNames);
}
#endif

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
  const bool result = wxApp::OnCmdLineParsed(parser);
  
  for (size_t i = 0; i < parser.GetParamCount(); i++)
  {
    m_Files.Add(parser.GetParam(i));
  }
  
  if (parser.Found("l"))
  {
    wxLogMessage(
      "Locale: " + GetLocale().GetLocale() + " dir: " + GetCatalogDir());
  }
  else if (parser.Found("v"))
  {
    wxLogMessage(wxExGetVersionInfo().GetVersionOnlyString());
  }

  return result;
}

bool App::OnInit()
{
  // This must be the first statement, other methods might use the name.
  SetAppName("syncped");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  Frame* frame = new Frame(m_Files);
  frame->Show();
  
  return true;
}

void App::OnInitCmdLine(wxCmdLineParser& parser)
{
  wxApp::OnInitCmdLine(parser);

  parser.AddParam(
    _("input file:line number:column number"),
    wxCMD_LINE_VAL_STRING,
    wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
    
  parser.AddSwitch("l", wxEmptyString, _("show locale"));
  parser.AddSwitch("v", wxEmptyString, _("show version"));
}
