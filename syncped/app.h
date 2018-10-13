////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'app'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/app.h>

class app : public wex::app
{
public:
  auto& GetData() {return m_Data;};
  const auto& GetFiles() const {return m_Files;};
  const auto& GetTag() const {return m_Tag;};

  auto& GetScriptin() {return m_Scriptin;};
  auto& GetScriptout() {return m_Scriptout;};

  auto GetDebug() const {return m_Debug;};
  auto GetSplit() const {return m_Split;};
  
  void Reset();
private:
#ifdef __WXOSX__  
  virtual void MacOpenFiles(const wxArrayString& fileNames) override;
#endif
  
  virtual bool OnInit() override;

  std::string m_Tag; 
  std::vector< wex::path > m_Files;
  
  bool m_Debug = false;
  int m_Split;
  wex::stc_data m_Data;
  wex::file m_Scriptin, m_Scriptout;
};
