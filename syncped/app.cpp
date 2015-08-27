////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class App
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
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
  wxExOpenFiles(frame, wxExToVectorString(fileNames).Get());
}
#endif

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
  const bool result = wxApp::OnCmdLineParsed(parser);
  
  for (size_t i = 0; i < parser.GetParamCount(); i++)
  {
    m_Files.push_back(parser.GetParam(i));
  }
  
  if (parser.Found("c"))
  {
    parser.Found("c", &m_Command);
  }
  
  if (parser.Found("l"))
  {
    wxLogMessage("Catalog dir: %s\nName: %s\nCanonical name: %s\nLanguage: %d\nLocale: %s\nIs ok: %d\nIs available: %d",
      GetCatalogDir().c_str(),
      GetLocale().GetName().c_str(),
      GetLocale().GetCanonicalName().c_str(),
      GetLocale().GetLanguage(), 
      GetLocale().GetLocale().c_str(),
      GetLocale().IsOk(),
      GetLocale().IsAvailable(GetLocale().GetLanguage())); 
  }
  
  if (parser.Found("o"))
  {
    m_Split = wxBOTTOM;
  }
  
  if (parser.Found("O"))
  {
    m_Split = wxRIGHT;
  }
  
  if (parser.Found("v"))
  {
    wxLogMessage(wxExGetVersionInfo().GetVersionOnlyString());
  }

  return result;
}

bool App::OnInit()
{
  // This must be the first statement, other methods might use the name.
  SetAppName("syncped");
  
  m_Split = -1;

  if (!wxExApp::OnInit())
  {
    return false;
  }

  Frame* frame = new Frame(this);
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
    
  parser.AddOption("c", wxEmptyString, _("command"), wxCMD_LINE_VAL_STRING);
  parser.AddSwitch("l", wxEmptyString, _("show locale"));
  parser.AddSwitch("o", wxEmptyString, _("split tabs horizontally"));
  parser.AddSwitch("O", wxEmptyString, _("split tabs vertically"));
  parser.AddSwitch("v", wxEmptyString, _("show version"));
}

void App::Reset()
{
  m_Command.clear();
  m_Split = -1;
}
