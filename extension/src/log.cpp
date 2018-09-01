////////////////////////////////////////////////////////////////////////////////
// Name:      log.cpp
// Purpose:   Implementation of class wxExLog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/log.h>
#include <wx/extension/item.h>
#include <wx/extension/listitem.h>
#include <easylogging++.h>

wxExLog::wxExLog(const std::string& topic, wxExLogLevel level)
  : m_Level(level)
  , m_Separator(!topic.empty())
{
  if (!topic.empty())
  {
    m_ss << topic << ":";
  }
}

wxExLog::wxExLog(wxExLogLevel level)
  : m_Level(level)
  , m_Separator(false)
{
}

wxExLog::wxExLog(const std::exception& e)
{
  m_ss << "std::exception:" << S() << e.what();
}

wxExLog::wxExLog(const pugi::xpath_exception& e)
{
  m_ss << "pugi::exception:" << S() << e.what();
}

wxExLog::wxExLog(const pugi::xml_parse_result& r)
{
  if (r.status != pugi::xml_parse_status::status_ok)
  {
    m_ss << 
      "xml parse result:" << S() << r.description() << S() <<
      "at offset:" << S() << r.offset;
  }
  else 
  {
    m_Level = LEVEL_INFO;
    m_Separator = false;
  }
}

wxExLog::~wxExLog()
{
  if (!m_ss.str().empty())
  {
    Log();
  }
}

wxExLog& wxExLog::operator<<(int r)
{
  m_ss << S() << r;
  return *this;
}

wxExLog& wxExLog::operator<<(const char* r)
{
  m_ss << S() << r;
  return *this;
}

wxExLog& wxExLog::operator<<(const std::string& r)
{
  m_ss << S() << r;
  return *this;
}

wxExLog& wxExLog::operator<<(const std::stringstream& r)
{
  m_ss << S() << r.str();
  return *this;
}

wxExLog& wxExLog::operator<<(const pugi::xml_node& r)
{
  m_ss << S() << "at offset:" << S() << r.offset_debug();
  return *this;
}

wxExLog& wxExLog::operator<<(const wxExItem& r)
{
  m_ss << S() << "item:" << S() << r.Log().str();
  return *this;
}

wxExLog& wxExLog::operator<<(const wxExListItem& r)
{
  m_ss << S() << "list item:" << S() << r.GetFileName().Path().string();
  return *this;
}

void wxExLog::Log() const
{  
  switch (m_Level)
  {
    case LEVEL_INFO:
      VLOG(9) << m_ss.str();
      break;
    case LEVEL_DEBUG:
      LOG(DEBUG) << m_ss.str();
      break;
    case LEVEL_WARNING:
      LOG(WARNING) << m_ss.str();
      break;
    case LEVEL_ERROR:
      LOG(ERROR) << m_ss.str();
      break;
    case LEVEL_FATAL:
      LOG(FATAL) << m_ss.str();
      break;
  }
}  

const std::string wxExLog::S()
{  
  const std::string s(m_Separator ? " ": "");
  m_Separator = true;
  return s;
}
