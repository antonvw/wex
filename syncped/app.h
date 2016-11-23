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
  const auto& GetTag() const {return m_Tag;};

  auto& GetScriptin() {return m_Scriptin;};
  auto& GetScriptout() {return m_Scriptout;};

  auto GetDebug() const {return m_Debug;};
  auto GetFlags() const {return m_Flags;};
  auto GetSplit() const {return m_Split;};
  
  void Reset();
private:
#ifdef __WXOSX__  
  virtual void MacOpenFiles(const wxArrayString& fileNames) override;
#endif
  virtual bool OnInit() override;

  std::vector< std::string > m_Files;
  bool m_Debug = false;
  int m_Flags = 0;
  int m_Split;
  std::string m_Command, m_Tag; 
  wxExFile m_Scriptin, m_Scriptout;
};
