////////////////////////////////////////////////////////////////////////////////
// Name:      log.cpp
// Purpose:   Implementation of class wex::log
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wx/app.h>
#include <wx/log.h>

#include <iomanip>

namespace logging = boost::log;

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

wex::log::log(const std::string& topic, level_t level)
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

wex::log& wex::log::operator<<(char* r)
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

      m_ss << "\\x" << std::setfill('0') << std::setw(2) << std::hex
           << static_cast<int>(c) << std::setfill(f) << std::setw(w);
    }
  }

  m_ss << "\"";

  return *this;
}

wex::log& wex::log::operator<<(const std::stringstream& r)
{
  if (!r.str().empty())
  {
    m_ss << S() << r.str();
  }

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
  if (const auto& text(get()); !text.empty() || m_level == LEVEL_STATUS)
  {
    switch (m_level)
    {
      case LEVEL_DEBUG:
        BOOST_LOG_TRIVIAL(debug) << text;
        break;

      case LEVEL_ERROR:
        BOOST_LOG_TRIVIAL(error) << text;
        break;

      case LEVEL_FATAL:
        BOOST_LOG_TRIVIAL(fatal) << text;
        break;

      case LEVEL_INFO:
        BOOST_LOG_TRIVIAL(info) << text;
        break;

      case LEVEL_STATUS:
        // this is a wxMSW bug, crash in test -tc=wex::stc -sc=find
        if (text.find("%") == std::string::npos)
        {
          wxLogStatus(text.c_str());
        }
        break;

      case LEVEL_TRACE:
        BOOST_LOG_TRIVIAL(trace) << text;
        break;

      case LEVEL_WARNING:
        BOOST_LOG_TRIVIAL(warning) << text;
        break;

      default:
        assert(0);
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

std::string wex::log::get_level_info()
{
  std::stringstream help;
  help << "valid ranges: " << LEVEL_TRACE << "-" << LEVEL_FATAL;

  for (int i = LEVEL_TRACE; i <= LEVEL_FATAL; i++)
  {
    help << "\n"
         << i << " "
         << logging::trivial::to_string((logging::trivial::severity_level)i);
  }

  return help.str();
}

wex::log wex::log::info(const std::string& topic)
{
  return log(topic, LEVEL_INFO);
}

void wex::log::init(level_t loglevel, const std::string& default_logfile)
{
  if (m_initialized)
  {
    return;
  }

  set_level(loglevel);

  logging::add_common_attributes();

  logging::add_console_log(
    std::cout,
    logging::keywords::format = "%TimeStamp% [%Severity%] %Message%");

  const path logfile(
    config::dir(),
    wxTheApp->GetAppName().ToStdString() + ".log");

  logging::add_file_log(
    logging::keywords::file_name =
      default_logfile.empty() ? logfile.string() : default_logfile,
    logging::keywords::open_mode = std::ios_base::app,
    logging::keywords::format    = "%TimeStamp% [%Severity%] %Message%");

  m_initialized = true;
}

wex::log::level_t wex::log::level_t_def()
{
  return LEVEL_ERROR;
}

void wex::log::set_level(level_t loglevel)
{
  switch (loglevel)
  {
    case LEVEL_DEBUG:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::debug);
      break;

    case LEVEL_ERROR:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::error);
      break;

    case LEVEL_FATAL:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::fatal);
      break;

    case LEVEL_INFO:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info);
      break;

    case LEVEL_OFF:
      logging::core::get()->set_filter(
        logging::trivial::severity > logging::trivial::fatal);
      break;

    case LEVEL_TRACE:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace);
      break;

    case LEVEL_WARNING:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::warning);
      break;

    default:
      log("unsupported level, using error level");
      set_level(LEVEL_ERROR);
      return;
  }

  m_level_filter = loglevel;
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
