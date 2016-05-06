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
  const auto& GetCommand() const {return m_Command;};
  const auto& GetFiles() const {return m_Files;};
  auto GetFlags() const {return m_Flags;};
  auto& GetScriptin() {return m_Scriptin;};
  auto& GetScriptout() {return m_Scriptout;};
  auto GetSplit() const {return m_Split;};
  
  void Reset();
private:
#ifdef __WXOSX__  
  virtual void MacOpenFiles(const wxArrayString& fileNames) override;
#endif
  virtual bool OnInit() override;

  std::vector< wxString > m_Files;
  int m_Flags;
  int m_Split;
  wxString m_Command; 
  wxExFile m_Scriptin, m_Scriptout;
};
