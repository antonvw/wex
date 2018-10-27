////////////////////////////////////////////////////////////////////////////////
// Name:      vcs_command.cpp
// Purpose:   Implementation of wex::vcs_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vcscommand.h>

bool wex::vcs_command::IsAdd() const
{
  return 
    GetCommand(menu_command::INCLUDE_NONE) == "add";
}

bool wex::vcs_command::IsBlame() const
{
  return 
    GetCommand(menu_command::INCLUDE_NONE) == "annotate" ||
    GetCommand(menu_command::INCLUDE_NONE) == "blame" ||
    GetCommand(menu_command::INCLUDE_NONE) == "print";
}

bool wex::vcs_command::IsCheckout() const
{
  return 
    GetCommand(menu_command::INCLUDE_NONE) == "checkout" ||
    GetCommand(menu_command::INCLUDE_NONE) == "co";
}

bool wex::vcs_command::IsCommit() const
{
  return 
    GetCommand(menu_command::INCLUDE_NONE) == "commit" ||
    GetCommand(menu_command::INCLUDE_NONE) == "ci" ||
    GetCommand(menu_command::INCLUDE_NONE) == "delta";
}

bool wex::vcs_command::IsDiff() const
{
  return 
    GetCommand(menu_command::INCLUDE_NONE).find("diff") != std::string::npos;
}

bool wex::vcs_command::IsHistory() const
{
  return 
    GetCommand(menu_command::INCLUDE_NONE) == "log" ||
    GetCommand(menu_command::INCLUDE_NONE) == "prs" ||
    GetCommand(menu_command::INCLUDE_NONE) == "prt";
}

bool wex::vcs_command::IsOpen() const
{
  return
    GetCommand(menu_command::INCLUDE_NONE) == "cat" ||
    GetCommand(menu_command::INCLUDE_NONE) == "get" ||
    GetCommand(menu_command::INCLUDE_NONE) == "show" ||
    IsBlame() ||
    IsDiff() ||
    IsHistory();
}
