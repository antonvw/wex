////////////////////////////////////////////////////////////////////////////////
// Name:      log.cpp
// Purpose:   Implementation of class wxExLog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/log.h>
#include <easylogging++.h>

wxExLog::wxExLog(int level)
  : m_Level(level)
{
}

void wxExLog::Log(const std::stringstream& ss)
{  
  switch (m_Level)
  {
    case LEVEL_INFO:
      LOG(INFO) << ss.str();
      break;
    case LEVEL_DEBUG:
      LOG(DEBUG) << ss.str();
      break;
    case LEVEL_WARNING:
      LOG(WARNING) << ss.str();
      break;
    case LEVEL_ERROR:
      LOG(ERROR) << ss.str();
      break;
    case LEVEL_FATAL:
      LOG(FATAL) << ss.str();
      break;
  }
}  

void wxExLog::Log(const std::exception& e, const std::stringstream& ss)
{
  std::stringstream sse;
  sse << "exception: " << e.what() << " " << ss.str();
  Log(sse);
}
