////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.cpp
// Purpose:   Implementation of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/vcsentry.h>
#include <wx/extension/defs.h> // for VCS_MAX_COMMANDS
#include <wx/extension/menu.h>
#include <wx/extension/menus.h>
#include <wx/extension/shell.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/vcs.h>

wxExVCSEntry::wxExVCSEntry(
  const std::string& name,
  const std::string& admin_dir,
  const std::vector<wxExVCSCommand> & commands,
  int flags_location)
  : wxExProcess()
  , wxExMenuCommands(name, commands)
  , m_AdminDir(admin_dir)
  , m_FlagsLocation(flags_location)
{
}

wxExVCSEntry::wxExVCSEntry(const pugi::xml_node& node)
  : wxExProcess()
  , wxExMenuCommands(node)
  , m_AdminDir(node.attribute("admin-dir").value())
  , m_AdminDirIsTopLevel(
      strcmp(node.attribute("toplevel").value(), "true") == 0)
  , m_FlagsLocation(
      (strcmp(node.attribute("flags-location").value(), "prefix") == 0 ?
         VCS_FLAGS_LOCATION_PREFIX: VCS_FLAGS_LOCATION_POSTFIX))
  , m_MarginWidth(atoi(node.attribute("margin-width").value()))
  , m_PosBegin(node.attribute("pos-begin").value())
  , m_PosEnd(node.attribute("pos-end").value())
{
}

int wxExVCSEntry::BuildMenu(int base_id, wxExMenu* menu, bool is_popup) const
{
  return wxExMenus::BuildMenu(GetCommands(), base_id, menu, is_popup);
}

bool wxExVCSEntry::Execute(
  const std::string& args,
  const wxExLexer& lexer,
  long pflags,
  const std::string& wd)
{
  m_Lexer = lexer;
  
  std::string prefix;
  
  if (m_FlagsLocation == VCS_FLAGS_LOCATION_PREFIX)
  {
    prefix = wxConfigBase::Get()->Read(_("Prefix flags"));
    
    if (!prefix.empty())
    {
      prefix += " ";
    }
  }
  
  std::string subcommand;
  
  if (GetCommand().UseSubcommand())
  {
    subcommand = wxConfigBase::Get()->Read(_("Subcommand"));

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
      "-m \"" + wxExConfigFirstOf(_("Revision comment")) + "\" ";
  }

  std::string my_args(args);

  // If we specified help (flags), we do not need a file argument.      
  if (GetCommand().IsHelp() || flags.find("help") != std::string::npos)
  {
    my_args.clear();
  }

  return wxExProcess::Execute(
    wxConfigBase::Get()->Read(GetName(), GetName()).ToStdString() + " " + 
      prefix +
      GetCommand().GetCommand() + " " + 
      subcommand + flags + comment + my_args, 
    pflags,
    wd);
}

const std::string wxExVCSEntry::GetBranch() const
{
  if (GetName() == "git")
  { 
    if (wxExProcess p; p.Execute("git branch", PROCESS_EXEC_WAIT))
    {
      for (wxExTokenizer tkz(p.GetStdOut(), "\r\n"); tkz.HasMoreTokens(); )
      {
        if (const auto token(tkz.GetNextToken()); token.find('*') == 0)
        {
          return wxExSkipWhiteSpace(token.substr(1));
        }
      }
    }
  }

  return std::string();
}

const std::string wxExVCSEntry::GetFlags() const
{
  return wxConfigBase::Get()->Read(_("Flags")).ToStdString();
}

void wxExVCSEntry::ShowOutput(const std::string& caption) const
{
  if (!GetError() && GetShell() != nullptr)
  {
    if (GetFlags().find("xml") != std::string::npos)
    {
      GetShell()->GetLexer().Set("xml");
    }
    else
    {
      wxExVCSCommandOnSTC(
        GetCommand(), 
        m_Lexer, 
        GetShell());
    }
  }

  wxExProcess::ShowOutput(caption);
}
