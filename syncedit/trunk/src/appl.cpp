/******************************************************************************\
* File:          appl.cpp
* Purpose:       Implementation of class 'Application'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: appl.cpp 49 2008-11-12 19:08:42Z anton $
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
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
  
  const int fontsizes[] = {4, 6, 8, 10, 12, 16, 22};
  GetPrinter()->SetFonts(wxEmptyString, wxEmptyString, fontsizes);
  GetPrinter()->GetPageSetupData()->SetMarginBottomRight(wxPoint(15, 5));
  GetPrinter()->GetPageSetupData()->SetMarginTopLeft(wxPoint(15, 5));
  
  exSTC::SetAllowSync(false);
  
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

  wxArrayString files;

  if (parser.GetParamCount() > 0)
  {
    for (size_t i = 0; i < parser.GetParamCount(); i++)
    {
      files.Add(parser.GetParam(i));
    }
  }

  MDIFrame* frame = new MDIFrame(files.GetCount() == 0);
  
  if (files.GetCount() > 0)
  {
    frame->Freeze();
    ftOpenFiles(frame, files);
    frame->Thaw();
  }

  frame->Show();
  
  wxUpdateUIEvent::SetUpdateInterval(0);
  
  exSTC::SetAllowSync(true);

  return true;
}
