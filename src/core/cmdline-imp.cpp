////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline-imp.cpp
// Purpose:   Implementation of wex::cmdline_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/log.h>
#include <wex/tokenizer.h>
#include <wex/version.h>
#include <wx/app.h>

#include "cmdline-imp.h"

#define WEX_CALLBACK(TYPE, FIELD)        \
  v->second.FIELD(it.second.as<TYPE>()); \
  if (data.save())                       \
    m_cfg.item(before(it.first, ',')).set(it.second.as<TYPE>());

namespace logging = boost::log;

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
  m_desc.add_options()("help,h", "displays usage information and exits");

  if (add_standard_options)
  {
    m_desc.add_options()("version", "displays version information and exits")(
      "level,V",
      po::value<int>()->default_value(4),
      "activates verbosity down to verbose level (valid range: 1-6)")(
      "verbose,v",
      "activates maximum (trace) verbosity")(
      "logfile,D",
      po::value<std::string>(),
      "sets log file");
  }
}

void wex::cmdline_imp::add_function(
  const std::string& name,
  const function_t&  t)
{
  m_functions.insert({before(name, ','), t});
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

  if (m_vm.count("help") || m_vm.count("version"))
  {
    std::stringstream ss;

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
        ss << m_desc;
        data.help(ss.str());
      }
      else
      {
        data.help(get_version_info().get());
      }
    }

    return false;
  }
  else if (m_vm.count("verbose"))
  {
    logging::core::get()->set_filter(
      logging::trivial::severity >= logging::trivial::trace);
  }
  else if (m_vm.count("level"))
  {
    if (const int level = m_vm["level"].as<int>(); level < 1 || level > 6)
    {
      std::cout << "unsupported level\n";
      return false;
    }
    else
      switch (level)
      {
        case 1:
          logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::trace);
          break;
        case 2:
          logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::debug);
          break;
        case 3:
          logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::info);
          break;
        case 4:
          logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::warning);
          break;
        case 5:
          logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::error);
          break;
        case 6:
          logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::fatal);
          break;
      }
  }

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
                m_cfg.item(before(it.first, ',')).set(val);
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

  log::init(
    m_vm.count("logfile") ? m_vm["logfile"].as<std::string>() : std::string());

  return true;
}
