////////////////////////////////////////////////////////////////////////////////
// Name:      vcs_entry.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vcsentry.h>
#include <wex/config.h>
#include <wex/defs.h> // for VCS_MAX_COMMANDS
#include <wex/menu.h>
#include <wex/menus.h>
#include <wex/shell.h>
#include <wex/tokenizer.h>
#include <wex/vcs.h>

wex::vcs_entry::vcs_entry(
  const std::string& name,
  const std::string& admin_dir,
  const std::vector<vcs_command> & commands,
  int flags_location)
  : process()
  , menu_commands(name, commands)
  , m_AdminDir(admin_dir)
  , m_FlagsLocation(flags_location)
{
}

wex::vcs_entry::vcs_entry(const pugi::xml_node& node)
  : process()
  , menu_commands(node)
  , m_AdminDir(node.attribute("admin-dir").value())
  , m_AdminDirIsTopLevel(
      strcmp(node.attribute("toplevel").value(), "true") == 0)
  , m_FlagsLocation(
      (strcmp(node.attribute("flags-location").value(), "prefix") == 0 ?
         FLAGS_LOCATION_PREFIX: FLAGS_LOCATION_POSTFIX))
  , m_MarginWidth(atoi(node.attribute("margin-width").value()))
  , m_PosBegin(node.attribute("pos-begin").value())
  , m_PosEnd(node.attribute("pos-end").value())
{
}

int wex::vcs_entry::BuildMenu(int base_id, menu* menu, bool is_popup) const
{
  return menus::BuildMenu(GetCommands(), base_id, menu, is_popup);
}

bool wex::vcs_entry::Execute(
  const std::string& args,
  const lexer& lexer,
  long pflags,
  const std::string& wd)
{
  m_Lexer = lexer;
  
  std::string prefix;
  
  if (m_FlagsLocation == FLAGS_LOCATION_PREFIX)
  {
    prefix = config(_("Prefix flags")).get();
    
    if (!prefix.empty())
    {
      prefix += " ";
    }
  }
  
  std::string subcommand;
  
  if (GetCommand().UseSubcommand())
  {
    subcommand = config(_("Subcommand")).get();

    if (!subcommand.empty())
    {
      subcommand += " ";
    }
  }

  std::string flags;

  if (GetCommand().AskFlags())
  {
    flags = GetFlags();
  }
  else if (GetCommand().GetFlags() != "none")
  {
    flags = GetCommand().GetFlags();
  }
  
  // E.g. in git you can do
  // git show HEAD~15:syncped/frame.cpp
  // where flags is HEAD~15:,
  // so there should be no space after it
  if (!flags.empty() && flags.back() != ':')
  {
    flags += " ";
  }

  std::string comment;

  if (GetCommand().IsCommit())
  {
    comment = 
      "-m \"" + config(_("Revision comment")).firstof() + "\" ";
  }

  std::string my_args(args);

  // If we specified help (flags), we do not need a file argument.      
  if (GetCommand().IsHelp() || flags.find("help") != std::string::npos)
  {
    my_args.clear();
  }

  return process::Execute(
    config(GetName()).get(GetName()) + " " + 
      prefix +
      GetCommand().GetCommand() + " " + 
      subcommand + flags + comment + my_args, 
    pflags,
    wd);
}

const std::string wex::vcs_entry::GetBranch() const
{
  if (GetName() == "git")
  { 
    if (process p; p.Execute("git branch", process::EXEC_WAIT))
    {
      for (tokenizer tkz(p.GetStdOut(), "\r\n"); tkz.HasMoreTokens(); )
      {
        if (const auto token(tkz.GetNextToken()); token.find('*') == 0)
        {
          return skip_white_space(token.substr(1));
        }
      }
    }
  }

  return std::string();
}

const std::string wex::vcs_entry::GetFlags() const
{
  return config(_("Flags")).get();
}

void wex::vcs_entry::ShowOutput(const std::string& caption) const
{
  if (!GetError() && GetShell() != nullptr)
  {
    if (GetFlags().find("xml") != std::string::npos)
    {
      GetShell()->GetLexer().Set("xml");
    }
    else
    {
      vcs_command_stc(
        GetCommand(), 
        m_Lexer, 
        GetShell());
    }
  }

  wex::process::ShowOutput(caption);
}
