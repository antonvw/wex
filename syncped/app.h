////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'App'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
