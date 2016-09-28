////////////////////////////////////////////////////////////////////////////////
// Name:      vcscommand.cpp
// Purpose:   Implementation of wxExVCSCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/vcscommand.h>

wxExVCSCommand::wxExVCSCommand()
  : m_Command()
  , m_SubMenu()
  , m_SubMenuIsCommand(false)
  , m_Type(VCS_COMMAND_IS_NONE)
{
}

wxExVCSCommand::wxExVCSCommand(
  const std::string& command,
  const std::string& type,
  const std::string& submenu,
  const std::string& subcommand)
  : m_Command(command)
  , m_SubMenu(!submenu.empty() ? submenu: subcommand)
  , m_SubMenuIsCommand(!subcommand.empty())
  , m_Type(From(type))
{
}
  
long wxExVCSCommand::From(const std::string& type) const
{
  long command = VCS_COMMAND_IS_BOTH;
  
  if (type.find("popup") != std::string::npos)
  {
    command = VCS_COMMAND_IS_POPUP;
  }
  else if (type.find("main") != std::string::npos)
  {
    command = VCS_COMMAND_IS_MAIN;
  }
  
  const long flags = (type.find("separator") != std::string::npos ? 
    VCS_COMMAND_SEPARATOR: 0);
  
  return command | flags;
}

const std::string wxExVCSCommand::GetCommand(
  bool include_subcommand,
  bool include_accelerators) const
{
  std::string command = m_Command;

  if (m_SubMenuIsCommand && include_subcommand)
  {
    command += " " + m_SubMenu;
  }

  if (command.find("&") != std::string::npos && !include_accelerators)
  {
    command.erase(
      std::remove(command.begin(), command.end(), '&'), command.end());
  }

  return command;
}

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

bool wxExVCSCommand::IsHelp() const
{
  return 
    GetCommand(false) == "help";
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

bool wxExVCSCommand::IsUpdate() const
{
  return 
    GetCommand(false) == "update" ||
    GetCommand(false) == "up";
}

bool wxExVCSCommand::UseFlags() const 
{
  // All commands, except help support flags.
  return !IsHelp();
}

bool wxExVCSCommand::UseSubcommand() const
{
  return IsHelp() || (m_SubMenuIsCommand && !m_SubMenu.empty());
}
