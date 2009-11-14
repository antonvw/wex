/******************************************************************************\
* File:          app.h
* Purpose:       Declaration of class 'Application'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _APP_H
#define _APP_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/app.h>

class Application : public wxExApp
{
private:
  virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
  virtual bool OnInit();
  virtual void OnInitCmdLine(wxCmdLineParser& parser);

  wxArrayString m_Files;
};
#endif
