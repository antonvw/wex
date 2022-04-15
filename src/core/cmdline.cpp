////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.cpp
// Purpose:   Implementation of wex::cmdline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>

#include "cmdline-imp.h"

#include <algorithm>
#include <iostream>
#include <numeric>

namespace wex
{
const std::string def_option(const std::vector<std::string>& v)
{
  return v.size() > 2 ? v.back() : std::string();
}

const std::string def_switch(const std::vector<std::string>& v)
{
  return v.size() > 2 ? v.back() : std::string("0");
}

template <typename T>
const auto find_if(const T& t, const std::vector<std::string>& v)
{
  return std::find_if(
    t.begin(),
    t.end(),
    [v](auto const& i)
    {
      return v[0] == find_before(i.first[0], ",");
    });
}

std::string get_option(
  const std::pair<
    const std::vector<std::string>,
    std::pair<cmdline::option_t, std::function<void(const std::any& any)>>>& p,
  config* cfg)
{
  const auto& name(find_before(p.first[0], ","));
  const auto& key(p.first[1]);

  try
  {
    switch (p.second.first)
    {
      case cmdline::FLOAT:
        return name + "=" +
               std::to_string(cfg->item(key).get(
                 static_cast<float>(std::stod(def_option(p.first))))) +
               "\n";

      case cmdline::INT:
        return name + "=" +
               std::to_string(
                 cfg->item(key).get(std::stoi(def_option(p.first)))) +
               "\n";

      case cmdline::STRING:
        return name + "=" + cfg->item(key).get(def_option(p.first)) + "\n";

      default:
        assert(0);
    }
  }
  catch (std::exception& e)
  {
    log(e) << "get_option:" << name << key;
  }

  return std::string();
}

std::string get_switch(
  const std::pair<const std::vector<std::string>, std::function<void(bool on)>>&
          p,
  config* cfg)
{
  const auto& name(find_before(p.first[0], ","));
  const auto& key(p.first[1]);

  try
  {
    return !cfg->item(key).get(
             static_cast<bool>(std::stoi(def_switch(p.first)))) ?
             "no" + name + "\n" :
             name + "\n";
  }
  catch (std::exception& e)
  {
    log(e) << "get_switch:" << name << key;
  }

  return std::string();
}
}; // namespace wex

#define ADD(TYPE, CONV)                        \
  if (!def_option(it.first).empty())           \
    m_parser->m_desc.add_options()(            \
      it.first[p_n].c_str(),                   \
      po::value<TYPE>()->implicit_value(CONV), \
      it.first[p_d].c_str());                  \
  else                                         \
    m_parser->m_desc.add_options()(            \
      it.first[p_n].c_str(),                   \
      po::value<TYPE>(),                       \
      it.first[p_d].c_str());

wex::cmdline::cmdline(
  const cmd_switches_t& s,
  const cmd_options_t&  o,
  const cmd_params_t&   p,
  bool                  add_standard_options,
  const std::string&    prefix)
  : m_cfg(new config(prefix))
  , m_options(o)
  , m_params(p)
  , m_switches(s)
  , m_add_standard_options(add_standard_options)
{
  if (!prefix.empty())
  {
    m_cfg->child_start();
  }
}

wex::cmdline::~cmdline()
{
  delete m_cfg;
  delete m_parser;
}

void wex::cmdline::get_all(std::string& help) const
{
  help += std::accumulate(
            m_options.begin(),
            m_options.end(),
            std::string(),
            [this](const std::string& a, const auto& b)
            {
              return a + get_option(b, m_cfg);
            }) +
          std::accumulate(
            m_switches.begin(),
            m_switches.end(),
            std::string(),
            [this](const std::string& a, const auto& b)
            {
              return a + get_switch(b, m_cfg);
            });
}

std::string wex::cmdline::get_output()
{
  return cmdline_imp::m_output;
}

std::string wex::cmdline::get_scriptout()
{
  return cmdline_imp::m_scriptout;
}

bool wex::cmdline::get_single(
  const std::vector<std::string>& v,
  std::string&                    help) const
{
  if (const auto& it = find_if(m_options, v); it != m_options.end())
  {
    help += get_option(*it, m_cfg);
    return true;
  }

  if (const auto& it = find_if(m_switches, v); it != m_switches.end())
  {
    help += get_switch(*it, m_cfg);
    return true;
  }

  return false;
}

bool wex::cmdline::is_echo()
{
  return cmdline_imp::m_is_echo;
}

bool wex::cmdline::is_output()
{
  return cmdline_imp::m_is_output;
}

void wex::cmdline::init()
{
  try
  {
    const size_t p_n{0}, p_d{1}; // par name, description

    std::for_each(
      m_switches.begin(),
      m_switches.end(),
      [this](const auto& it)
      {
        m_parser->m_desc.add_options()(
          it.first[p_n].c_str(),
          po::bool_switch(),
          it.first[p_d].c_str());
        m_parser->add_function(it.first[p_n], it.second);
      });

    std::for_each(
      m_options.begin(),
      m_options.end(),
      [this](const auto& it)
      {
        m_parser->add_function(
          it.first[p_n],
          {it.second.second, it.second.first});

        switch (it.second.first)
        {
          case FLOAT:
            ADD(float, (float)std::stod(def_option(it.first)));
            break;
          case INT:
            ADD(int, std::stoi(def_option(it.first)));
            break;
          case STRING:
            ADD(std::string, def_option(it.first));
            break;
          default:
            assert(0);
        }
      });

    if (!m_params.first.first.empty())
    {
      m_parser->m_desc.add_options()(
        m_params.first.first.c_str(),
        po::value<std::vector<std::string>>(),
        m_params.first.second.c_str());
      m_parser->m_pos_desc.add(m_params.first.first.c_str(), -1);
      m_parser->add_function(m_params.first.first, m_params.second);
    }
  }
  catch (std::exception& e)
  {
    log(e) << "cmdline";
  }
}

bool wex::cmdline::parse(data::cmdline& data)
{
  try
  {
    if (m_parser == nullptr)
    {
      m_parser = new cmdline_imp(m_add_standard_options, *m_cfg);
      init();
    }

    return m_parser->parse(data);
  }
  catch (std::exception& e)
  {
    if (data.av() != nullptr)
    {
      std::cout << e.what() << "\n";
    }
    else
    {
      data.help(e.what());
    }
  }
  catch (...)
  {
    log("unsupported flags");
  }

  return false;
}

bool wex::cmdline::parse_set(data::cmdline& data) const
{
  // [option[=[value]] ...][nooption ...][option? ...][all]
  /*
    When no arguments are specified, write the value of the term edit
    option and those options whose values have been changed from the default
    settings; when the argument all is specified, write all of the option
    values.

    Giving an option name followed by the character '?' shall cause the
    current value of that option to be written.
    The '?' can be separated from the option name by zero or more <blank>
    characters. The '?' shall be necessary only for Boolean valued options.
    Boolean options can be given values by the form set option to turn
    them on or set no option to turn them off; string and numeric options
    can be assigned by the form set option= value.
    Any <blank> characters in strings can be included as is by preceding
    each <blank> with an escaping <backslash>.
    More than one option can be set or listed by a single set command
    by specifying multiple arguments, each separated from the next by one
    or more <blank> characters.
    */
  bool  found = false;
  regex r(
    {{"all",
      [&, this](const regex::match_t& m)
      {
        std::string help;
        get_all(help);
        data.help(help);
      }},
     // [nooption ...]
     {"no([a-z0-9]+)(.*)",
      [&, this](const regex::match_t& m)
      {
        if (set_no_option(m, data.save()))
          found = true;
      }},
     // [option? ...]
     {"([a-z0-9]+)[ \t]*\\?(.*)",
      [&, this](const regex::match_t& m)
      {
        std::string help;
        if (get_single(m, help))
        {
          data.help(help);
          found = true;
        }
      }},
     // [option[=[value]] ...]
     {"([a-z0-9]+)(=[a-z0-9/.]+)?(.*)",
      [&, this](const regex::match_t& m)
      {
        if (set_option(m, data.save()))
          found = true;
      }}});

  try
  {
    for (auto line(boost::algorithm::trim_copy(data.string())); !line.empty();
         line = r.matches().back())
    {
      switch (r.search(line); r.which_no())
      {
        case -1:
          data.help("unmatched cmdline: " + line);
          return false;

        case 0:
          return true;
      }
    }
  }
  catch (std::exception& e)
  {
    data.help(e.what());
  }

  return found;
}

bool wex::cmdline::set_no_option(const std::vector<std::string>& v, bool save)
  const
{
  return set_option_check(v, save, false);
}

bool wex::cmdline::set_option(const std::vector<std::string>& v, bool save)
  const
{
  if (v.size() > 1 && v[1][0] == '=')
  {
    // skip the =
    const auto val(v[1].substr(1));

    for (const auto& it : m_options)
    {
      if (v[0] == find_before(it.first[0], ","))
      {
        switch (it.second.first)
        {
          case wex::cmdline::FLOAT:
            m_cfg->item(it.first[1]).set(std::stof(val));
            if (it.second.second != nullptr)
              it.second.second(std::stof(val));
            return true;

          case wex::cmdline::INT:
            m_cfg->item(it.first[1]).set(std::stoi(val));
            if (it.second.second != nullptr)
              it.second.second(std::stoi(val));
            return true;

          case wex::cmdline::STRING:
            m_cfg->item(it.first[1]).set(val);
            if (it.second.second != nullptr)
              it.second.second(val);
            return true;

          default:
            assert(0);
        }
      }
    }
  }
  else
  {
    return set_option_check(v, save, true);
  }

  return false;
}

bool wex::cmdline::set_option_check(
  const std::vector<std::string>& v,
  bool                            save,
  bool                            check) const
{
  if (const auto& it = find_if(m_switches, v); it != m_switches.end())
  {
    if (save)
    {
      m_cfg->item(it->first[1]).set(check);
    }

    if (it->second != nullptr)
    {
      it->second(check);
    }

    return true;
  }

  return false;
}

bool wex::cmdline::use_events()
{
  return cmdline_imp::m_use_events;
}
