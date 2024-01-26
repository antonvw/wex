////////////////////////////////////////////////////////////////////////////////
// Name:      command-parser.cpp
// Purpose:   Implementation of class wex::command_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/command-parser.h>
#include <wex/factory/control.h>
#include <wex/syntax/stc.h>

wex::command_parser::command_parser(
  ex*                ex,
  const std::string& text,
  parse_t            type)
  : m_text(text)
  , m_ex(ex)
{
  m_is_ok = m_text.empty() ? false : parse(type);
}

bool wex::command_parser::parse(parse_t type)
{
  if (m_text.starts_with(ex_command::selection_range()))
  {
    if (!parse_selection())
    {
      return false;
    }
  }
  else if (!parse_other())
  {
    return false;
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
        const auto line(address(m_ex, m_text).get_line());
        return m_ex->get_stc()->inject(data::control().line(line));
      }

      case address_t::ONE_ADDR:
        return address(m_ex).parse(*this);

      case address_t::TWO_ADDR:
        if (info_message_t im; !addressrange(m_ex, m_range).parse(*this, im))
        {
          return false;
        }
        else if (im != info_message_t::NONE)
        {
          m_ex->info_message(m_ex->register_text(), im);
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

bool wex::command_parser::parse_other()
{
  marker_and_register_expansion(m_ex, m_text);

  // Addressing in ex.
  const std::string addr(
    // (1) . (2) $ (3) decimal number, + or - (7)
    "[\\.\\$0-9\\+\\-]+|"
    // (4) marker
    "'[a-z]|"
    // (5) (6) regex find, non-greedy!
    "[\\?/].*?[\\?/]");

  const auto& cmds_1addr(address(m_ex).regex_commands());
  const auto& cmds_2addr(addressrange(m_ex).regex_commands());

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
                  log::trace("ex 1addr") << m_range;
                }},
               // 2addr
               {"^(" + addr + "),(" + addr + ")" + cmds_2addr,
                [&](const regex::match_t& m)
                {
                  m_type  = address_t::TWO_ADDR;
                  m_range = m[0] + "," + m[1];
                  m_cmd   = m[2];
                  m_text  = m[3];
                  log::trace("ex 2addr") << m_range;
                }},
               // 2addr
               {"^(" + addr + ")?" + cmds_2addr,
                [&](const regex::match_t& m)
                {
                  m_type  = address_t::TWO_ADDR;
                  m_range = m[0];
                  m_cmd   = m[1];
                  m_text  = m[2];
                  log::trace("ex 2addr") << m_range;
                }}});
      v.match(m_text) <= 1)
  {
    m_type = address_t::NO_ADDR;
  }

  if (m_range.empty() && m_cmd != "!")
  {
    m_range =
      (m_cmd.starts_with("g") || m_cmd == "v" || m_cmd == "w" ? "%" : ".");
  }

  return true;
}

bool wex::command_parser::parse_selection()
{
  if (m_ex->get_stc()->get_selected_text().empty())
  {
    return false;
  }

  const auto& cmds_2addr(addressrange(m_ex).regex_commands());

  if (regex r(
        {{ex_command::selection_range() + cmds_2addr,
          [&](const regex::match_t& m)
          {
            m_type  = address_t::TWO_ADDR;
            m_range = ex_command::selection_range();
            m_cmd   = m[0];
            m_text  = m[1];
            log::trace("ex selection") << m_range;
          }}});
      r.match(m_text) == 2)
  {
    return true;
  }

  return false;
}
