////////////////////////////////////////////////////////////////////////////////
// Name:      vcs_command.cpp
// Purpose:   Implementation of wex::vcs_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/vcscommand.h>

bool wex::vcs_command::IsAdd() const
{
  return 
    GetCommand(false) == "add";
}

bool wex::vcs_command::IsBlame() const
{
  return 
    GetCommand(false) == "annotate" ||
    GetCommand(false) == "blame" ||
    GetCommand(false) == "print";
}

bool wex::vcs_command::IsCheckout() const
{
  return 
    GetCommand(false) == "checkout" ||
    GetCommand(false) == "co";
}

bool wex::vcs_command::IsCommit() const
{
  return 
    GetCommand(false) == "commit" ||
    GetCommand(false) == "ci" ||
    GetCommand(false) == "delta";
}

bool wex::vcs_command::IsDiff() const
{
  return 
    GetCommand(false).find("diff") != std::string::npos;
}

bool wex::vcs_command::IsHistory() const
{
  return 
    GetCommand(false) == "log" ||
    GetCommand(false) == "prs" ||
    GetCommand(false) == "prt";
}

bool wex::vcs_command::IsOpen() const
{
  return
    GetCommand(false) == "cat" ||
    GetCommand(false) == "get" ||
    GetCommand(false) == "show" ||
    IsBlame() ||
    IsDiff() ||
    IsHistory();
}
