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

  exApp::OnInit();
  
  SetLogging();

  static const wxCmdLineEntryDesc cmdLineDesc[] =
  {
    {wxCMD_LINE_SWITCH, "h", "help",  "show this help",  
      wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    {wxCMD_LINE_PARAM, NULL, NULL, "input file:line number", 
       wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},
    {wxCMD_LINE_NONE}
  };

  wxCmdLineParser parser(cmdLineDesc, argc, argv);

  if (parser.Parse() != 0)
  {
    return false;
  }

  MDIFrame* frame = new MDIFrame(parser.GetParamCount() == 0);
  frame->Show();  
  
  if (parser.GetParamCount() > 0)
  {
    wxArrayString files;

    for (size_t i = 0; i < parser.GetParamCount(); i++)
    {
      files.Add(parser.GetParam(i));
    }

    ftOpenFiles(frame, files);
  }

  wxUpdateUIEvent::SetUpdateInterval(0);
  
  return true;
}
