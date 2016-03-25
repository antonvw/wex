////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'App'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
  virtual void MacOpenFiles(const wxArrayString& fileNames) override;
#endif
  virtual bool OnInit() override;

  std::vector< wxString > m_Files;
  wxString m_Command;
  int m_Flags;
  int m_Split;
};
