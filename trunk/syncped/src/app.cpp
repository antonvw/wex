/******************************************************************************\
* File:          app.cpp
* Purpose:       Implementation of class 'App'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/cmdline.h> // for wxCmdLineParser
#include <wx/extension/filename.h>
#include <wx/extension/util.h>
#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(App);

#ifdef __WXOSX__  
void App::MacOpenFile(const wxString& fileName)
{
  Frame* frame = (Frame*)GetTopWindow();
  frame->OpenFile(wxExFileName(fileName));
}
#endif

bool App::OnCmdLineParsed(wxCmdLineParser& parser)
{
  for (size_t i = 0; i < parser.GetParamCount(); i++)
  {
    m_Files.Add(parser.GetParam(i));
  }

  return wxApp::OnCmdLineParsed(parser);
}

bool App::OnInit()
{
  // This must be the first statement, other methods might use the name.
  SetAppName("syncped");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  Frame* frame = new Frame(m_Files.Count() == 0);
  frame->Show();

  wxExOpenFiles(frame, m_Files, 0, wxDIR_FILES); // only files in this dir
 
  return true;
}

void App::OnInitCmdLine(wxCmdLineParser& parser)
{
  wxApp::OnInitCmdLine(parser);

  parser.AddParam(
    "input file:line number",
    wxCMD_LINE_VAL_STRING,
    wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
}
