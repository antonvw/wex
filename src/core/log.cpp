////////////////////////////////////////////////////////////////////////////////
// Name:      log.cpp
// Purpose:   Implementation of class wex::log
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2023 Anton van Wezenbeek
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

#include <codecvt>
#include <iomanip>

namespace logging = boost::log;

namespace wex
{
std::string get_logfile(const std::string& default_logfile)
{
  if (!default_logfile.empty())
  {
    return default_logfile;
  }

  assert(wxTheApp != nullptr);

  const wex::path p(
    config::dir(),
    wxTheApp->GetAppName().ToStdString() + ".log");

  return p.string();
}

std::string quote(const std::string& r, log::level_t level)
{
  if (level == log::level_t::STATUS)
  {
    return "";
  }
  else
  {
    return r.contains(" ") ? "\"" : "";
  }
}

std::string ws2s(const std::wstring& wstr)
{
  using convert_typeX = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_typeX, wchar_t> converterX;

  return converterX.to_bytes(wstr);
}
} // namespace wex

wex::log::log(const std::string& topic)
  : log(topic, level_t::ERRORS)
{
}

wex::log::log(const std::exception& e)
  : log(std::string(), level_t::ERRORS)
{
  m_ss << "std::exception:" << S() << e.what();
}

wex::log::log(const pugi::xml_parse_result& r)
  : log(std::string(), level_t::ERRORS)
{
  if (r.status != pugi::xml_parse_status::status_ok)
  {
    m_ss << "xml parse result:" << S() << r.description() << S()
         << "at offset:" << S() << r.offset;
  }
  else
  {
    m_level     = level_t::INFO;
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

wex::log& wex::log::operator<<(unsigned int r)
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
  m_ss << S() << ws2s(r);
  return *this;
}

wex::log& wex::log::operator<<(const std::string& r)
{
  m_ss << S() << quote(r, m_level);

  if (r.empty())
  {
    m_ss << "<empty>";
  }
  else
  {
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
  }

  m_ss << quote(r, m_level);

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
  return log(topic, level_t::DEBUG);
}

wex::log wex::log::fatal(const std::string& topic)
{
  return log(topic, level_t::FATAL);
}

void wex::log::flush()
{
  if (const auto& text(get()); !text.empty() || m_level == level_t::STATUS)
  {
    switch (m_level)
    {
      case level_t::DEBUG:
        BOOST_LOG_TRIVIAL(debug) << text;
        break;

      case level_t::ERRORS:
        BOOST_LOG_TRIVIAL(error) << text;
        break;

      case level_t::FATAL:
        BOOST_LOG_TRIVIAL(fatal) << text;
        break;

      case level_t::INFO:
        BOOST_LOG_TRIVIAL(info) << text;
        break;

      case level_t::OFF:
        break;

      case level_t::STATUS:
        if (m_level_filter != level_t::OFF)
        {
          wxLogStatus("%s", text.c_str());
        }
        break;

      case level_t::TRACE:
        BOOST_LOG_TRIVIAL(trace) << text;
        break;

      case level_t::WARNING:
        BOOST_LOG_TRIVIAL(warning) << text;
        break;

      default:
        assert(0);
    }
  }
}

const std::string wex::log::get() const
{
  return (!m_topic.empty() && !m_ss.str().empty() ? m_topic + ":" : m_topic) +
         m_ss.str();
}

std::string wex::log::get_level_info()
{
  std::stringstream help;
  help << "valid ranges: " << std::to_underlying(level_t::TRACE) << "-"
       << std::to_underlying(level_t::OFF);

  for (int i = std::to_underlying(level_t::TRACE);
       i <= std::to_underlying(level_t::FATAL);
       i++)
  {
    help << "\n"
         << i << " "
         << logging::trivial::to_string((logging::trivial::severity_level)i);
  }

  help << "\n"
       << std::to_underlying(level_t::STATUS) << " "
       << "status";
  help << "\n"
       << std::to_underlying(level_t::OFF) << " "
       << "none";

  return help.str();
}

wex::log wex::log::info(const std::string& topic)
{
  return log(topic, level_t::INFO);
}

wex::log::level_t wex::log::level_t_def()
{
  return level_t::ERRORS;
}

void wex::log::on_init(level_t loglevel, const std::string& default_logfile)
{
  if (m_initialized)
  {
    return;
  }

  set_level(loglevel);
  m_logfile = get_logfile(default_logfile);

  logging::add_common_attributes();

  logging::add_console_log(
    std::cout,
    logging::keywords::format = "%TimeStamp% [%Severity%] %Message%");

  logging::add_file_log(
    logging::keywords::file_name = m_logfile,
    logging::keywords::open_mode = std::ios_base::app,
    logging::keywords::format    = "%TimeStamp% [%Severity%] %Message%");

  m_initialized = true;
}

const std::string wex::log::path()
{
  return m_logfile;
}

void wex::log::set_level(level_t loglevel)
{
  switch (loglevel)
  {
    case level_t::DEBUG:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::debug);
      break;

    case level_t::ERRORS:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::error);
      break;

    case level_t::FATAL:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::fatal);
      break;

    case level_t::INFO:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info);
      break;

    case level_t::OFF:
    case level_t::STATUS:
      logging::core::get()->set_filter(
        logging::trivial::severity > logging::trivial::fatal);
      break;

    case level_t::TRACE:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace);
      break;

    case level_t::WARNING:
      logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::warning);
      break;

    default:
      log("unsupported level, using error level");
      set_level(level_t::ERRORS);
      return;
  }

  m_level_filter = loglevel;
}

wex::log wex::log::status(const std::string& topic)
{
  return log(topic, level_t::STATUS);
}

wex::log wex::log::trace(const std::string& topic)
{
  return log(topic, level_t::TRACE);
}

wex::log wex::log::warning(const std::string& topic)
{
  return log(topic, level_t::WARNING);
}

const std::string wex::log::S()
{
  const std::string s(m_separator ? " " : "");
  m_separator = true;
  return s;
}
