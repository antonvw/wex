////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline-imp.cpp
// Purpose:   Implementation of wex::cmdline_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/tokenize.h>
#include <wex/core/version.h>
#include <wx/app.h>
#include <wx/timer.h>
#include <wx/window.h>

#include "cmdline-imp.h"

#include <iostream>
#include <sstream>

#define WEX_CALLBACK(TYPE, FIELD)        \
  v->second.FIELD(it.second.as<TYPE>()); \
  if (data.save())                       \
    m_cfg.item(find_before(it.first, ",")).set(it.second.as<TYPE>());

wex::cmdline_imp::function_t::function_t(
  std::function<void(const std::any&)> f,
  cmdline::option_t                    o)
  : m_fo(f)
  , m_type(F_OPTION)
  , m_type_o(o)
{
}

wex::cmdline_imp::function_t::function_t(std::function<void(bool)> f)
  : m_fs(f)
  , m_type(F_SWITCH)
{
}

wex::cmdline_imp::function_t::function_t(
  std::function<void(std::vector<std::string>)> f)
  : m_fp(f)
  , m_type(F_PARAM)
{
}

wex::cmdline_imp::cmdline_imp(bool add_standard_options, config& cfg)
  : m_desc()
  , m_cfg(cfg)
{
  if (add_standard_options)
  {
    // clang-format off
    // options are sorted on short option if present, long option
    m_desc.add_options()
      ("logfile,D", po::value<std::string>(), "sets log file")
      ("echo,e", "echo commands")
      ("help,h", "displays usage information and exits")
      ("quit,q", po::value<int>(),
         "quits after specified number of milliseconds")
      ("version,r", "displays version information and exits")
      ("verbose,v", "activates maximum (trace) verbosity")
      ("level,V", po::value<int>()->default_value(log::level_t_def()),
        std::string("activates verbosity down from verbose level\n" +
          log::get_level_info()).c_str())
      ("scriptout,w", po::value<std::string>(), 
         "script out append (echo to file <arg>)")
      ("echo-output,x", "echo output commands (process, statusbar)")
      ("output,X", po::value<std::string>(), "output commands append to file");
    // clang-format on
  }
  else
  {
    m_desc.add_options()("help,h", "displays usage information and exits");
  }
}

void wex::cmdline_imp::add_function(
  const std::string& name,
  const function_t&  t)
{
  m_functions.insert({find_before(name, ","), t});
}

bool wex::cmdline_imp::parse(data::cmdline& data)
{
  data.av() != nullptr ?
    po::store(
      po::command_line_parser(data.ac(), data.av())
        .options(m_desc)
        .positional(m_pos_desc)
        .run(),
      m_vm) :
    po::store(
      po::command_line_parser(tokenize<std::vector<std::string>>(data.string()))
        .options(m_desc)
        .positional(m_pos_desc)
        .run(),
      m_vm);

  po::notify(m_vm);

  auto loglevel = m_vm.count("level") ? m_vm["level"].as<int>() :
                                        static_cast<int>(log::level_t_def());

  if (m_vm.count("help") || m_vm.count("version"))
  {
    parse_help(data);
    return false;
  }

  if (m_vm.count("echo"))
  {
    m_is_echo = true;
  }

  if (m_vm.count("echo-output"))
  {
    m_is_output = true;
  }

  if (m_vm.count("output"))
  {
    m_output = m_vm["output"].as<std::string>();
  }

  if (m_vm.count("quit"))
  {
    parse_quit();
  }

  if (m_vm.count("scriptout"))
  {
    m_scriptout = m_vm["scriptout"].as<std::string>();
  }

  if (m_vm.count("verbose"))
  {
    loglevel = log::LEVEL_TRACE;
  }

  if (!parse_args(data))
  {
    return false;
  }

  log::init(
    (log::level_t)loglevel,
    m_vm.count("logfile") ? m_vm["logfile"].as<std::string>() : std::string());

  return true;
}

bool wex::cmdline_imp::parse_args(const data::cmdline& data)
{
  for (const auto& it : m_vm)
  {
    if (!it.second.defaulted())
    {
      try
      {
        if (const auto& v = m_functions.find(it.first); v != m_functions.end())
        {
          switch (v->second.m_type)
          {
            case function_t::F_OPTION:
              switch (v->second.m_type_o)
              {
                case cmdline::FLOAT:
                  WEX_CALLBACK(float, m_fo);
                  break;

                case cmdline::INT:
                  WEX_CALLBACK(int, m_fo);
                  break;

                case cmdline::STRING:
                  WEX_CALLBACK(std::string, m_fo);
                  break;
              }
              break;

            case function_t::F_PARAM:
              v->second.m_fp(it.second.as<std::vector<std::string>>());
              break;

            case function_t::F_SWITCH:
              bool val(it.second.as<bool>());

              v->second.m_fs(val);

              if (data.save())
              {
                m_cfg.item(find_before(it.first, ",")).set(val);
              }
              break;
          }
        }
      }
      catch (std::exception& e)
      {
        log(e) << "parser" << it.first;
        return false;
      }
    }
  }

  return true;
}

void wex::cmdline_imp::parse_help(data::cmdline& data)
{
  if (data.av() != nullptr)
  {
    if (m_vm.count("help"))
      std::cout << m_desc;
    else
      std::cout << wxTheApp->GetAppName() << " " << get_version_info().get()
                << "\n";
  }
  else
  {
    if (m_vm.count("help"))
    {
      std::stringstream ss;
      ss << m_desc;
      data.help(ss.str());
    }
    else
    {
      data.help(get_version_info().get());
    }
  }
}

void wex::cmdline_imp::parse_quit()
{
  if (const auto quit(m_vm["quit"].as<int>()); quit > 0)
  {
    m_use_events = true;

    const auto id_quit     = wxWindowBase::NewControlId();
    auto*      timer_start = new wxTimer(wxTheApp, id_quit);

    timer_start->StartOnce(quit);

    wxTheApp->Bind(
      wxEVT_TIMER,
      [=, this](wxTimerEvent& event)
      {
        if (auto* win = wxTheApp->GetTopWindow(); win != nullptr)
        {
          win->Destroy();
        }
        else
        {
          wxExit();
        }
      },
      id_quit);
  }
}
