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

#include <wx/cmdline.h> // for wxCmdLineParser
#include <wx/extension/log.h>
#include "app.h"
#include "frame.h"

DECLARE_APP(App)
IMPLEMENT_APP(App)

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

  if (!wxConfigBase::Get()->Exists(_("List Colour")))
  {
    wxConfigBase::Get()->Write(_("List Colour"), wxColour("RED"));
  }

  wxExProcessWithListView::InitCommandFromConfig();

  wxExLog::Get()->SetLogging();

  MDIFrame* frame = new MDIFrame(m_Files.Count() == 0);
  frame->Show();

  SetTopWindow(frame);

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
