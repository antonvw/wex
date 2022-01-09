////////////////////////////////////////////////////////////////////////////////
// Name:      command-parser.cpp
// Purpose:   Implementation of class wex::command_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/log.h>
#include <wex/data/control.h>
#include <wex/factory/stc.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/command-parser.h>

#include "util.h"

wex::command_parser::command_parser(
  ex*                ex,
  const std::string& text,
  parse_t            type)
  : m_text(text)
{
  m_is_ok = m_text.empty() ? false : parse(ex, type);
}

bool wex::command_parser::parse(ex* ex, parse_t type)
{
  if (m_text.starts_with("'<,'>"))
  {
    if (ex->get_stc()->get_selected_text().empty())
    {
      return false;
    }

    m_type  = address_t::TWO_ADDR;
    m_range = "'<,'>";
    m_cmd   = m_text.substr(5);
    m_text  = m_text.substr(6);
  }
  else
  {
    marker_and_register_expansion(ex, m_text);

    // Addressing in ex.
    const std::string addr(
      // (1) . (2) $ (3) decimal number, + or - (7)
      "[\\.\\$0-9\\+\\-]+|"
      // (4) marker
      "'[a-z]|"
      // (5) (6) regex find, non-greedy!
      "[\\?/].*?[\\?/]");

    const auto& cmds_1addr(address(ex).regex_commands());
    const auto& cmds_2addr(addressrange(ex).regex_commands());

    if (regex v({// 2addr % range
                 {"^%" + cmds_2addr,
                  [&](const regex::match_t& m)
                  {
                    m_type  = address_t::TWO_ADDR;
                    m_range = "%";
                    m_cmd   = m[0];
                    m_text  = m[1];
                  }},
                 // 1addr (or none)
                 {"^(" + addr + ")?" + cmds_1addr,
                  [&](const regex::match_t& m)
                  {
                    m_type  = address_t::ONE_ADDR;
                    m_range = m[0];
                    m_cmd   = m[1];
                    m_text  = boost::algorithm::trim_left_copy(m[2]);
                  }},
                 // 2addr
                 {"^(" + addr + ")?(," + addr + ")?" + cmds_2addr,
                  [&](const regex::match_t& m)
                  {
                    m_type  = address_t::TWO_ADDR;
                    m_range = m[0] + m[1];
                    m_cmd   = m[2];
                    m_text  = m[3];
                  }}});
        v.match(m_text) <= 1)
    {
      m_type = address_t::NO_ADDR;
    }

    if (m_range.empty() && m_cmd != '!')
    {
      m_range =
        (m_cmd.starts_with("g") || m_cmd == 'v' || m_cmd == 'w' ? "%" : ".");
    }
  }

  if (type == parse_t::CHECK)
  {
    return true;
  }

  try
  {
    switch (m_type)
    {
      case address_t::NO_ADDR:
      {
        m_range.clear();
        const auto line(address(ex, m_text).get_line());
        return ex->get_stc()->inject(data::control().line(line));
      }

      case address_t::ONE_ADDR:
        return address(ex).parse(*this);

      case address_t::TWO_ADDR:
        if (info_message_t im; !addressrange(ex, m_range).parse(*this, im))
        {
          return false;
        }
        else if (im != info_message_t::NONE)
        {
          ex->info_message(ex->register_text(), im);
        }
        break;

      default:
        assert(0);
    }
  }
  catch (std::exception& e)
  {
    log(e) << m_cmd;
    return false;
  }

  return true;
}
