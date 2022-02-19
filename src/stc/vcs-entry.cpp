////////////////////////////////////////////////////////////////////////////////
// Name:      vcs-entry.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/stc/shell.h>
#include <wex/stc/vcs-entry.h>
#include <wex/ui/menu.h>
#include <wex/ui/menus.h>

wex::vcs_entry::vcs_entry(const pugi::xml_node& node)
  : process()
  , menu_commands(node)
  , m_admin_dir(node.attribute("admin-dir").value())
  , m_flags_location(
      (strcmp(node.attribute("flags-location").value(), "prefix") == 0 ?
         FLAGS_LOCATION_PREFIX :
         FLAGS_LOCATION_POSTFIX))
  , m_blame(node)
  , m_log_flags(node.attribute("log-flags").value())
{
}

const std::string wex::vcs_entry::bin() const
{
  return !config("vcs." + name()).get(name()).empty() ?
           config("vcs." + name()).get(name()) :
           name();
}

size_t wex::vcs_entry::build_menu(int base_id, menu* menu) const
{
  return menus::build_menu(get_commands(), base_id, menu);
}

bool wex::vcs_entry::execute(
  const std::string& args,
  const lexer&       lexer,
  const std::string& wd)
{
  m_lexer = lexer;

  std::string prefix;

  if (m_flags_location == FLAGS_LOCATION_PREFIX)
  {
    prefix = config(_("vcs.Prefix flags")).get();

    if (!prefix.empty())
    {
      prefix += " ";
    }
  }

  std::string subcommand;

  if (get_command().use_subcommand())
  {
    subcommand = config(_("vcs.Subcommand")).get();

    if (!subcommand.empty())
    {
      subcommand += " ";
    }
  }

  std::string flags;

  if (get_command().ask_flags())
  {
    flags = get_flags();
  }
  else if (get_command().flags() != "none")
  {
    flags = get_command().flags();
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

  if (get_command().is_commit())
  {
    comment =
      "-m \"" + config(_("vcs.Revision comment")).get_first_of() + "\" ";
  }

  std::string my_args(args);

  // If we specified help (flags), we do not need a file argument.
  if (get_command().is_help() || flags.find("help") != std::string::npos)
  {
    my_args.clear();
  }

  return process::system(process_data(
                           bin() + " " + prefix + get_command().get_command() +
                           " " + subcommand + flags + comment + my_args)
                           .start_dir(wd)) == 0;
}

bool wex::vcs_entry::execute(const std::string& command, const std::string& wd)
{
  // Get flags if available.
  std::string flags;
  std::string cmd(command);

  if (const vcs_command & vc(find(get_word(cmd))); !vc.get_command().empty())
  {
    flags = " " + vc.flags();
  }

  return process::system(
           process_data(bin() + " " + command + flags).start_dir(wd)) == 0;
}

const std::string wex::vcs_entry::get_branch(const std::string& wd) const
{
  if (name() == "git")
  {
    if (process p; p.system(process_data(bin() + " branch").start_dir(wd)) == 0)
    {
      for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
             p.std_out(),
             boost::char_separator<char>("\r\n")))
      {
        if (it.starts_with('*'))
        {
          return boost::algorithm::trim_copy(it.substr(1));
        }
      }
    }
  }

  return std::string();
}

const std::string wex::vcs_entry::get_flags() const
{
  return config(_("vcs.Flags")).get();
}

bool wex::vcs_entry::log(const path& p, const std::string& id)
{
  if (m_log_flags.empty())
  {
    wex::log("log flags empty") << name();
    return false;
  }

  const std::string command =
    m_flags_location == FLAGS_LOCATION_PREFIX || name() == "svn" ?
      bin() + " log " + m_log_flags + " " + id :
      bin() + " log " + id + " " + m_log_flags;

  return process::system(process_data(command).start_dir(p.parent_path())) == 0;
}

void wex::vcs_entry::show_output(const std::string& caption) const
{
  if (!std_out().empty() && get_shell() != nullptr)
  {
    if (get_flags().find("xml") != std::string::npos)
    {
      get_shell()->get_lexer().set("xml");
    }
    else
    {
      vcs_command_stc(get_command(), m_lexer, get_shell());
    }
  }

  if (get_shell() != nullptr)
  {
    get_shell()->clear();
  }

  wex::process::show_output(caption);
}
