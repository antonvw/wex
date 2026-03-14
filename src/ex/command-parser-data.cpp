////////////////////////////////////////////////////////////////////////////////
// Name:      command-parser-data.cpp
// Purpose:   Implementation of class wex::command_parser
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/command-parser-data.h>

#include <utility>

wex::command_parser_data::command_parser_data(std::string text)
  : m_text(std::move(text))
{
}

bool wex::command_parser_data::is_global_skip() const
{
  return m_cmd == "m" || m_cmd == "t";
}
