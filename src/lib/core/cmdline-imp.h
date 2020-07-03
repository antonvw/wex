////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline-imp.h
// Purpose:   Declaration of wex::cmdline_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/program_options.hpp>
#include <wex/cmdline.h>
#include <wex/config.h>

namespace po = boost::program_options;

namespace wex
{
  /// Returns default option value.
  const std::string def_option(const std::vector<std::string> v);

  /// Returns default switch value.
  const std::string def_switch(const std::vector<std::string> v);

  /// Support class to implement program options.
  class cmdline_imp
  {
    friend class cmdline;

  public:
    // Support struct.
    struct function_t
    {
      enum tag_t
      {
        F_OPTION,
        F_SWITCH,
        F_PARAM
      };

      function_t(std::function<void(const std::any&)> f, cmdline::option_t o);
      function_t(std::function<void(bool)> f);
      function_t(std::function<void(std::vector<std::string>)> f);

      const tag_t             m_type;
      const cmdline::option_t m_type_o = cmdline::STRING;

      const std::function<void(const std::any&)>          m_fo;
      const std::function<void(std::vector<std::string>)> m_fp;
      const std::function<void(bool)>                     m_fs;
    };

    /// Constructor.
    cmdline_imp(bool add_standard_options, config& cfg);

    /// Add function.
    void add_function(const std::string& name, const function_t& t);

    /// Parse line from arg count and array.
    bool parse(int ac, char* av[], bool save);

    /// Parse line from string.
    bool parse(const std::string& s, std::string& help, bool save);

  private:
    bool parse_handle(std::string* help, bool save);

    std::map<std::string, function_t> m_functions;

    po::options_description            m_desc;
    po::positional_options_description m_pos_desc;
    po::variables_map                  m_vm;

    config& m_cfg;
  };
}; // namespace wex
