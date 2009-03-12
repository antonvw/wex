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
#include <wx/stdpaths.h>
#include "appl.h"
#include "frame.h"

DECLARE_APP(Application)
IMPLEMENT_APP(Application)

bool Application::OnInit()
{
  // This must be the first statement, other methods might use the name.
#ifdef EX_PORTABLE
  SetAppName("syncped");
#else
  SetAppName("syncedit");
#endif

  // Otherwise wxGTK gives problems.
  wxUpdateUIEvent::SetUpdateInterval(-1);

  if (!exApp::OnInit())
  {
    return false;
  }
  
  SetLogging();

  MDIFrame* frame = new MDIFrame(m_Files.Count() == 0);

  frame->Show();  

  if (m_Files.Count() > 0)
  {
    ftOpenFiles(frame, m_Files);
  }

  wxUpdateUIEvent::SetUpdateInterval(0);
  
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

bool Application::OnCmdLineParsed(wxCmdLineParser& parser)
{
  if (parser.GetParamCount() > 0)
  {
  
    for (size_t i = 0; i < parser.GetParamCount(); i++)
    {
      m_Files.Add(parser.GetParam(i));
    }
  }

  return wxApp::OnCmdLineParsed(parser);
}
