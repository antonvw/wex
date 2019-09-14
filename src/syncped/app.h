////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'app'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/app.h>

class app : public wex::app
{
public:
  auto& data() {return m_Data;};
  auto  get_debug() const {return m_Debug;};
  auto& get_files() const {return m_Files;};
  auto& get_scriptin() {return m_Scriptin;};
  auto& get_scriptout() {return m_Scriptout;};
  auto  get_split() const {return m_Split;};
  auto& get_tag() const {return m_Tag;};
  
  void reset();
private:
#ifdef __WXOSX__  
  virtual void MacOpenFiles(const wxArrayString& fileNames) override;
#endif
  
  virtual bool OnInit() override;

  std::string m_Tag, m_Scriptin, m_Scriptout; 
  std::vector< wex::path > m_Files;
  
  bool m_Debug {false};
  int m_Split;
  wex::stc_data m_Data;
};
