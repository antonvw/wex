/******************************************************************************\
* File:          app.h
* Purpose:       Declaration of class 'App'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _APP_H
#define _APP_H

#include <wx/extension/app.h>

class App : public wxExApp
{
private:
#ifdef __WXOSX__  
  virtual void MacOpenFile(const wxString& fileName);
#endif
  virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
  virtual bool OnInit();
  virtual void OnInitCmdLine(wxCmdLineParser& parser);

  wxArrayString m_Files;
};
#endif
