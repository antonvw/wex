////////////////////////////////////////////////////////////////////////////////
// Name:      vcscommand.cpp
// Purpose:   Implementation of wxExVCSCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/vcscommand.h>

bool wxExVCSCommand::IsAdd() const
{
  return 
    GetCommand(false) == "add";
}

bool wxExVCSCommand::IsBlame() const
{
  return 
    GetCommand(false) == "annotate" ||
    GetCommand(false) == "blame" ||
    GetCommand(false) == "print";
}

bool wxExVCSCommand::IsCheckout() const
{
  return 
    GetCommand(false) == "checkout" ||
    GetCommand(false) == "co";
}

bool wxExVCSCommand::IsCommit() const
{
  return 
    GetCommand(false) == "commit" ||
    GetCommand(false) == "ci" ||
    GetCommand(false) == "delta";
}

bool wxExVCSCommand::IsDiff() const
{
  return 
    GetCommand(false).find("diff") != std::string::npos;
}

bool wxExVCSCommand::IsHistory() const
{
  return 
    GetCommand(false) == "log" ||
    GetCommand(false) == "prs" ||
    GetCommand(false) == "prt";
}

bool wxExVCSCommand::IsOpen() const
{
  return
    GetCommand(false) == "cat" ||
    GetCommand(false) == "get" ||
    GetCommand(false) == "show" ||
    IsBlame() ||
    IsDiff() ||
    IsHistory();
}
