////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'App'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _APP_H
#define _APP_H

#include <wx/extension/app.h>

class App : public wxExApp
{
private:
#ifdef __WXOSX__  
  virtual void MacOpenFiles(const wxArrayString& fileNames);
#endif
  virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
  virtual bool OnInit();
  virtual void OnInitCmdLine(wxCmdLineParser& parser);

  std::vector< wxString > m_Files;
  int m_Split;
};
#endif
