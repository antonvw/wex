////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'App'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/app.h>

class App : public wxExApp
{
public:
  const wxString& GetCommand() const {return m_Command;};
  const std::vector< wxString > & GetFiles() const {return m_Files;};
  int GetFlags() const {return m_Flags;};
  int GetSplit() const {return m_Split;};
  
  void Reset();
private:
#ifdef __WXOSX__  
  virtual void MacOpenFiles(const wxArrayString& fileNames);
#endif
  virtual bool OnInit();

  std::vector< wxString > m_Files;
  wxString m_Command;
  int m_Flags;
  int m_Split;
};
