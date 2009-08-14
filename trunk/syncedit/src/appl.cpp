/******************************************************************************\
* File:          appl.cpp
* Purpose:       Implementation of class 'Application'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/cmdline.h> // for wxCmdLineParser
#include "appl.h"
#include "frame.h"

DECLARE_APP(Application)
IMPLEMENT_APP(Application)

bool Application::OnCmdLineParsed(wxCmdLineParser& parser)
{
  for (size_t i = 0; i < parser.GetParamCount(); i++)
  {
    m_Files.Add(parser.GetParam(i));
  }

  return wxApp::OnCmdLineParsed(parser);
}

bool Application::OnInit()
{
  // This must be the first statement, other methods might use the name.
#ifdef EX_PORTABLE
  SetAppName("syncped");
#else
  SetAppName("syncedit");
#endif

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExProcessWithListView::InitCommandFromConfig();

  SetLogging();

  MDIFrame* frame = new MDIFrame(m_Files.Count() == 0);
  frame->Show();

  SetTopWindow(frame);

  wxExOpenFiles(frame, m_Files, 0, wxDIR_FILES); // only files in this dir
 
  return true;
}

void Application::OnInitCmdLine(wxCmdLineParser& parser)
{
  wxApp::OnInitCmdLine(parser);

  parser.AddParam(
    "input file:line number",
    wxCMD_LINE_VAL_STRING,
    wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
}
