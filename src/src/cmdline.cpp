////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.cpp
// Purpose:   Implementation of wex::cmdline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <boost/program_options.hpp>
#include <wx/app.h>
#include <wx/versioninfo.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/tokenizer.h>
#include <wex/util.h>
#include <wex/version.h>

namespace po = boost::program_options;

#define WEX_CALLBACK(TYPE, FIELD)                                    \
  v->second.FIELD(it.second.as<TYPE>());                             \
  if (save) config(before(it.first, ',')).set(it.second.as<TYPE>()); \

namespace wex
{
  class cmdline_imp
  {
    friend class cmdline;
  public:
    struct function_t
    {
      enum tag_t
      {
        F_OPTION,
        F_SWITCH,
        F_PARAM
      };

      function_t(std::function<void(const std::any&)> f, cmdline::option_t o)
        : m_fo(f)
        , m_type(F_OPTION)
        , m_type_o(o) {;};
      function_t(std::function<void(bool)> f)
        : m_fs(f)
        , m_type(F_SWITCH) {;};
      function_t(std::function<void(std::vector<std::string> )> f)
        : m_fp(f)
        , m_type(F_PARAM) {;};
      
      const tag_t m_type;
      const cmdline::option_t m_type_o = cmdline::STRING;
      const std::function<void(const std::any&)> m_fo;
      const std::function<void(std::vector<std::string> )> m_fp;
      const std::function<void(bool)> m_fs;
    };

    cmdline_imp(const std::string& message)
      : m_desc(message)
    {
      m_desc.add_options()
        ("help,h", "displays usage information and exits")
        ("version", "displays version information and exits");
    };
    
    void add_function(
      const std::string& name, 
      const function_t& t, 
      cmdline::option_t c = cmdline::STRING)
    {
      m_functions.insert({before(name, ','), t});
    }
    
    bool parse(int ac, char* av[], bool save)
    {
      po::store(po::command_line_parser(ac, av).
        options(m_desc).positional(m_pos_desc).run(), m_vm);
      return parse_handle(nullptr, save);
    }

    bool parse(const std::string& s, std::string& help, bool save)
    {
      tokenizer tkz(s);
      const auto v(tkz.tokenize<std::vector<std::string>>());
      po::store(po::command_line_parser(v).
        options(m_desc).positional(m_pos_desc).run(), m_vm);
      return parse_handle(&help, save);
    }
  private:
    bool parse_handle(std::string* help, bool save)
    {
      po::notify(m_vm);  

      if (m_vm.count("help") || m_vm.count("version")) 
      {
        std::stringstream ss;

        if (help == nullptr) 
        {
          if (m_vm.count("help"))
            std::cout << m_desc;
          else
            std::cout << wxTheApp->GetAppName() << " " << get_version_info().get() << "\n";
        }
        else 
        {
          if (m_vm.count("help"))
          {
            ss << m_desc;
            *help = ss.str();
          }
          else
          {
            *help = get_version_info().get();
          }
        }
        
        return false;
      }

      for (const auto& it : m_vm)
      {
        if (!it.second.defaulted())
        {
          try
          {
            if (auto v = m_functions.find(it.first); v != m_functions.end())
            {
              switch (v->second.m_type)
              {
                case function_t::F_OPTION:
                  switch (v->second.m_type_o)
                  {
                    case cmdline::FLOAT: WEX_CALLBACK(float, m_fo); break;
                    case cmdline::INT: WEX_CALLBACK(int, m_fo); break;
                    case cmdline::STRING: WEX_CALLBACK(std::string, m_fo); break;
                  }
                break;
                case function_t::F_PARAM:
                  v->second.m_fp(it.second.as<std::vector<std::string>>());
                break;
                case function_t::F_SWITCH: 
                  v->second.m_fs(cmdline::toggle() ? 
                    config(it.first).get(it.second.as<bool>()): 
                    it.second.as<bool>());
                  if (save) 
                  {
                    config(before(it.first, ',')).set(cmdline::toggle() ?
                      !config(it.first).get(it.second.as<bool>()):
                       it.second.as<bool>());
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

    std::map<std::string, function_t> m_functions; 
    po::options_description m_desc;
    po::positional_options_description m_pos_desc;
    po::variables_map m_vm;
  };
};

#define ADD(TYPE, CONV)                      \
  if (!def_specified.empty())                \
    m_parser->m_desc.add_options()           \
      (it->first[p_n].c_str(),               \
       po::value<TYPE>()->implicit_value(CONV), \
       it->first[p_d].c_str());              \
  else                                       \
    m_parser->m_desc.add_options()           \
      (it->first[p_n].c_str(),               \
       po::value<TYPE>(),                    \
       it->first[p_d].c_str());              \
             
wex::cmdline::cmdline(
  const cmd_switches_t & s, const cmd_options_t & o, const cmd_params_t & p,
  const std::string& message)
  : m_parser(new cmdline_imp(message))
{
  try
  {
    const size_t p_n{0}, p_d{1}; // par name, description

    for (auto it = s.begin(); it != s.end(); ++it)
    {
      if (m_toggle && m_first)
      {
        const std::string def_specified = 
          (it->first.size() > 2 ? it->first.back(): std::string("1"));
        config(before(it->first[p_n], ',')).set((bool)std::stoi(def_specified));
      }
      
      m_parser->m_desc.add_options()
         (it->first[p_n].c_str(), 
          po::bool_switch(),
          it->first[p_d].c_str());
      m_parser->add_function(it->first[p_n], it->second);
    }

    for (auto it = o.begin(); it != o.end(); ++it)
    {
      const std::string def_specified = 
        (it->first.size() > 2 ? it->first.back(): std::string());

      m_parser->add_function(it->first[p_n], {it->second.second, it->second.first});

      switch (it->second.first)
      {
        case FLOAT: ADD(float, (float)std::stod(def_specified)); break;
        case INT: ADD(int, std::stoi(def_specified)); break;
        case STRING: ADD(std::string, def_specified); break;
      }
    }

    if (!p.first.first.empty())
    {
      m_parser->m_desc.add_options()
         (p.first.first.c_str(), 
          po::value< std::vector<std::string> >(), 
          p.first.second.c_str());
      m_parser->m_pos_desc.add(p.first.first.c_str(), -1);
      m_parser->add_function(p.first.first, p.second);
    }
    
    m_first = false;
  }
  catch (std::exception& e)
  {
    log(e) << "cmdline";
  }
}

wex::cmdline::~cmdline()
{
  delete m_parser;
}
  
bool wex::cmdline::parse(int ac, char* av[], bool save) const
{
  try
  {
    return m_parser->parse(ac, av, save);
  }
  catch (std::exception& e)
  {
    std::cout << e.what() << "\n";
    return false;
  }
}

bool wex::cmdline::parse(
  const std::string& cmdline, std::string& help, bool save) const
{
  try
  {
    return m_parser->parse(cmdline, help, save);
  }
  catch (std::exception& e)
  {
    help = e.what();
    return false;
  }
}
