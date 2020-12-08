////////////////////////////////////////////////////////////////////////////////
// Name:      log.cpp
// Purpose:   Implementation of class wex::log
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <easylogging++.h>
#include <iomanip>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wx/log.h>

INITIALIZE_EASYLOGGINGPP

enum level_t
{
  // boost
  LEVEL_FATAL = 1,
  LEVEL_WARNING,
  LEVEL_INFO,
  LEVEL_DEBUG,
  LEVEL_TRACE,

  LEVEL_ERROR,
  // other levels
  LEVEL_STATUS,
};

const std::string level_string(int level)
{
  switch (level)
  {
    case LEVEL_DEBUG:
      return "debug";
    case LEVEL_ERROR:
      return "error";
    case LEVEL_FATAL:
      return "fatal";
    case LEVEL_INFO:
      return "info";
    case LEVEL_STATUS:
      return "status";
    case LEVEL_TRACE:
      return "trace";
    case LEVEL_WARNING:
      return "warning";
    default:
      return std::to_string(level);
  }
}

wex::log::log(const std::string& topic)
  : log(topic, LEVEL_ERROR)
{
}

wex::log::log(const std::exception& e)
  : log(std::string(), LEVEL_ERROR)
{
  m_ss << "std::exception:" << S() << e.what();
}

wex::log::log(const pugi::xml_parse_result& r)
  : log(std::string(), LEVEL_ERROR)
{
  if (r.status != pugi::xml_parse_status::status_ok)
  {
    m_ss << "xml parse result:" << S() << r.description() << S()
         << "at offset:" << S() << r.offset;
  }
  else
  {
    m_level     = LEVEL_INFO;
    m_separator = false;
  }
}

wex::log::log(const std::string& topic, int level)
  : m_level(level)
  , m_topic(topic)
  , m_separator(!topic.empty())
{
}

wex::log::~log()
{
  flush();
}

wex::log& wex::log::operator<<(char r)
{
  m_ss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(bool r)
{
  m_ss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(int r)
{
  m_ss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(size_t r)
{
  m_ss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(long r)
{
  m_ss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(long long r)
{
  m_ss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(const char* r)
{
  m_ss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(const wchar_t* r)
{
  m_wss << S() << r;
  return *this;
}

wex::log& wex::log::operator<<(const std::string& r)
{
  m_ss << S() << "\"";

  for (const auto& c : r)
  {
    if (isprint(c))
    {
      m_ss << c;
    }
    else
    {
      const char f(m_ss.fill());
      const auto w(m_ss.width());

      m_ss << "\\x" << std::setfill('0') << std::setw(2) << std::hex << (int)c
           << std::setfill(f) << std::setw(w);
    }
  }

  m_ss << "\"";

  return *this;
}

wex::log& wex::log::operator<<(const std::stringstream& r)
{
  m_ss << S() << r.str();
  return *this;
}

wex::log& wex::log::operator<<(const pugi::xml_node& r)
{
  m_ss << S() << "at offset:" << S() << r.offset_debug();
  return *this;
}

wex::log wex::log::debug(const std::string& topic)
{
  return log(topic, LEVEL_DEBUG);
}

wex::log wex::log::fatal(const std::string& topic)
{
  return log(topic, LEVEL_FATAL);
}

void wex::log::flush()
{
  if (const std::string text(get()); !text.empty() || m_level == LEVEL_STATUS)
  {
    switch (m_level)
    {
      case LEVEL_ERROR:
        LOG(ERROR) << text;
        break;

      case LEVEL_STATUS:
        // this is a wxMSW bug, crash in test -tc=wex::stc -sc=find
        if (text.find("%") == std::string::npos)
        {
          wxLogStatus(text.c_str());
        }
        break;

      default:
        VLOG(m_level) << text;
    }
  }
}

const std::string wex::log::get() const
{
  return (!m_topic.empty() && (!m_ss.str().empty() || !m_wss.str().empty()) ?
            m_topic + ":" :
            m_topic) +
         m_ss.str() + m_wss.str();
}

wex::log wex::log::info(const std::string& topic)
{
  return log(topic, LEVEL_INFO);
}

void wex::log::init(int argc, char** argv)
{
  // Load elp configuration from file.
  const path elp(config::dir(), "conf.elp");

  if (elp.file_exists())
  {
    el::Loggers::reconfigureAllLoggers(el::Configurations(elp.string()));
  }

  // We need to convert argc and argv, as elp expects = sign between values.
  // The logging-flags are handled by syncped.
  std::vector<std::string>                               v;
  const std::vector<std::pair<std::string, std::string>> supported{
    {"-m", "-vmodule"},
    {"-D", "--default-log-file"},
    {"-L", "--logging-flags"},
    {"-V", "--v"},
    {"--level", "--v"},
    {"--logfile", "--default-log-file"},
    {"--logflags", "--logging-flags"},
    {"--x", "--v"}}; // for testing with verbosity: --v=9

  for (int i = 0; i < argc; i++)
  {
    bool added = false;

    for (const auto& s : supported)
    {
      if (strcmp(argv[i], s.first.c_str()) == 0)
      {
        if (i + 1 < argc)
        {
          const std::string option(argv[i + 1]);
          v.push_back(std::string(std::string(s.second) + "=" + option));
          added = true;
          i++;
          break;
        }
      }
    }

    if (!added)
    {
      v.push_back(argv[i]);
    }
  }

  std::vector<char*> w;
  for (const auto& arg : v)
  {
    w.push_back((char*)arg.data());
  }
  w.push_back(nullptr);

  START_EASYLOGGINGPP(w.size() - 1, w.data());

  info("verbosity") << level_string(el::Loggers::verboseLevel());
  trace("log setup") << elp.string();
}

wex::log wex::log::status(const std::string& topic)
{
  return log(topic, LEVEL_STATUS);
}

wex::log wex::log::trace(const std::string& topic)
{
  return log(topic, LEVEL_TRACE);
}

wex::log wex::log::warning(const std::string& topic)
{
  return log(topic, LEVEL_WARNING);
}

const std::string wex::log::S()
{
  const std::string s(m_separator ? " " : "");
  m_separator = true;
  return s;
}
