////////////////////////////////////////////////////////////////////////////////
// Name:      vcs-command.cpp
// Purpose:   Implementation of wex::vcs_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/vcs-command.h>

bool wex::vcs_command::is_add() const
{
  return get_command(0) == "add";
}

bool wex::vcs_command::is_blame() const
{
  return get_command(0) == "annotate" || get_command(0) == "blame" ||
         get_command(0) == "print";
}

bool wex::vcs_command::is_checkout() const
{
  return get_command(0) == "checkout" || get_command(0) == "co";
}

bool wex::vcs_command::is_commit() const
{
  return get_command(0) == "commit" || get_command(0) == "ci" ||
         get_command(0) == "delta";
}

bool wex::vcs_command::is_diff() const
{
  return get_command(0).find("diff") != std::string::npos;
}

bool wex::vcs_command::is_history() const
{
  return get_command(0) == "log" || get_command(0) == "prs" ||
         get_command(0) == "prt";
}

bool wex::vcs_command::is_open() const
{
  return get_command(0) == "cat" || get_command(0) == "get" ||
         get_command() == "show" || is_blame() || is_diff() || is_history();
}
