////////////////////////////////////////////////////////////////////////////////
// Name:      vcs-entry.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2010-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log-none.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/stc/shell.h>
#include <wex/syntax/path-lexer.h>
#include <wex/ui/frame.h>
#include <wex/ui/menus.h>
#include <wex/vcs/unified-diff.h>
#include <wex/vcs/vcs-entry.h>
#include <wx/app.h>

#include "util.h"

namespace wex
{
std::set<wex::path>
parse(const path& toplevel, const std::string& file, const std::string& regex)
{
  std::set<wex::path> v;
  std::fstream fs(path(toplevel).append(path(file)).data(), std::ios_base::in);

  if (!fs.is_open())
  {
    wex::log::trace(file) << "not opened";
    return v;
  }

  for (std::string line; std::getline(fs, line);)
  {
    if (!line.empty())
    {
      if (wex::regex r(regex); r.match(line) > 0)
      {
        v.emplace(path(toplevel).append(
          path(r[0].ends_with("/") ? r[0].substr(0, r[0].size() - 1) : r[0])));
      }
    }
  }

  return v;
}
} // namespace wex

wex::vcs_entry::vcs_entry(const pugi::xml_node& node)
  : menu_commands(node)
  , m_admin_dir(node.attribute("admin-dir").value())
  , m_flags_location(
      (strcmp(node.attribute("flags-location").value(), "prefix") == 0 ?
         flags_location_t::PREFIX :
         flags_location_t::POSTFIX))
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
  const path&        p,
  const std::string& wd)
{
  m_lexer = path_lexer(p).lexer();
  const path& tl(factory::vcs_admin(admin_dir(), p).toplevel());

  if (p.file_exists() && get_command().get_command() == "show")
  {
    const std::string& repo_path(p.string().substr(tl.string().size() + 1));
    revisions_dialog(repo_path, tl, p);
    return false; // skip rest in vcs_execute
  }

  if (get_command().get_command() == "grep")
  {
    execute_grep(bin(), tl);
    return false; // skip rest in vcs_execute
  }

  std::string prefix;

  if (m_flags_location == flags_location_t::PREFIX)
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

  if (get_command().get_command() != "show")
  {
    if (get_command().get_command() == "diff")
    {
      flags = "-U0";
    }
    else if (get_command().ask_flags())
    {
      flags = get_flags();
    }
    else if (get_command().flags() != "none")
    {
      flags = get_command().flags();
    }

    if (!flags.empty())
    {
      flags += " ";
    }
  }

  std::string comment;

  if (get_command().is_commit())
  {
    comment =
      "-m \"" + config(_("vcs.Revision comment")).get_first_of() + "\" ";
  }

  std::string my_args(args);

  // If we specified help (flags), we do not need a file argument.
  if (get_command().is_help() || flags.contains("help"))
  {
    my_args.clear();
  }

  return process::system(process_data(
                           bin() + " " + prefix + get_command().get_command() +
                           " " + subcommand + flags + comment + my_args)
                           .start_dir(wd)) == 0;
}

const std::string wex::vcs_entry::get_branch(const std::string& wd) const
{
  wex::log_none off;

  if (process p; name() == "git" &&
                 p.system(process_data(bin() + " branch").start_dir(wd)) == 0)
  {
    const auto& tok(boost::tokenizer<boost::char_separator<char>>(
      p.std_out(),
      boost::char_separator<char>("\r\n")));

    if (const auto& it = std::ranges::find_if(
          tok,
          [](const auto& i)
          {
            return i.starts_with('*');
          });
        it != tok.end())
    {
      return boost::algorithm::trim_copy(it->substr(1));
    }
  }

  return std::string();
}

const std::string wex::vcs_entry::get_flags() const
{
  return config(flags_key()).get();
}

bool wex::vcs_entry::log(const path& p, const std::string& id)
{
  const std::string separator = (!m_log_flags.empty() ? " " : std::string());
  std::string       command   = bin() + " log ";

  command += m_flags_location == flags_location_t::PREFIX || name() == "svn" ?
               m_log_flags + separator + id :
               id + separator + m_log_flags;

  return process::system(process_data(command).start_dir(p.parent_path())) == 0;
}

std::optional<std::set<wex::path>>
wex::vcs_entry::setup_exclude(const path& toplevel, const path& p)
{
  if (name() != "git")
  {
    return {};
  }

  const std::string allowed("[0-9A-Za-z_\\-\\/]+");
  auto x(parse(toplevel, ".gitmodules", "\t+path = (" + allowed + ")"));
  auto y(parse(toplevel, ".gitignore", "(" + allowed + ")"));

  x.merge(y);

  return std::optional<std::set<wex::path>>{x};
}

void wex::vcs_entry::show_output(const std::string& caption) const
{
  if (!std_out().empty() && get_shell() != nullptr)
  {
    if (get_flags().contains("xml"))
    {
      get_shell()->get_lexer().set("xml");
    }
    else
    {
      vcs_command_stc(get_command(), m_lexer, get_shell());
    }

    if (vcs_diff(get_command().get_command()))
    {
      if (unified_diff ud(
            path(),
            this,
            dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow()));
          ud.parse() && ud.differences() == 0)
      {
        log::status("No output");
      }
      return;
    }
  }

  if (get_shell() != nullptr)
  {
    get_shell()->clear();
  }

  wex::process::show_output(caption);
}

int wex::vcs_entry::system(const process_data& data)
{
  std::string args;

  if (!data.args_str().empty())
  {
    args = data.args_str();
  }
  else
  {
    std::istringstream cmd(data.exe());
    args = cmd.str();
    std::string word;
    cmd >> word;

    if (const vcs_command & vc(find(word)); !vc.get_command().empty())
    {
      args += " " + vc.flags();
    }
  }

  return process::system(
    process_data(bin() + " " + args).start_dir(data.start_dir()));
}
