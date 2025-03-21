////////////////////////////////////////////////////////////////////////////////
// Name:      commands-ex.cpp
// Purpose:   Implementation of class wex::ex::commands_ex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <charconv>
#include <numeric>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wex/core/version.h>
#include <wex/ctags/ctags.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/path-lexer.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wx/app.h>

#define POST_COMMAND(ID)                                                       \
  {                                                                            \
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID);                     \
                                                                               \
    if (command.contains(" "))                                                 \
    {                                                                          \
      event.SetString(command.substr(command.find(' ') + 1));                  \
    }                                                                          \
                                                                               \
    wxPostEvent(wxTheApp->GetTopWindow(), event);                              \
  };

namespace wex
{
enum class command_arg_t
{
  INT,
  NONE,
  OTHER,
};

command_arg_t get_command_arg(const std::string& command)
{
  if (const auto& post(wex::find_after(command, " ")); post == command)
  {
    return command_arg_t::NONE;
  }
  /* NOLINTNEXTLINE */
  else if (int val;
           std::from_chars(post.data(), post.data() + post.size(), val).ec ==
           std::errc())
  {
    return command_arg_t::INT;
  }

  return command_arg_t::OTHER;
}

bool source(ex* ex, const std::string& cmd)
{
  if (!cmd.contains(" "))
  {
    return false;
  }

  wex::path path(wex::find_first_of(cmd, " "));

  if (path.is_relative())
  {
    path.make_absolute();
  }

  if (!path.file_exists())
  {
    return false;
  }

  wex::file script(path.data());
  bool      result = true;

  if (const auto buffer(script.read()); buffer != nullptr)
  {
    int i = 0;

    for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
           *buffer,
           boost::char_separator<char>("\r\n")))
    {
      if (const std::string & line(it); !line.empty())
      {
        if (line == cmd)
        {
          log(":so recursive line") << i + 1 << line;
          return false;
        }

        if (
          line.starts_with(":a") || line.starts_with(":i") ||
          line.starts_with(":c"))
        {
          if (!ex->command(line + "\n"))
          {
            log(":so command insert failed line") << i + 1 << line;
            result = false;
          }
        }
        else if (!ex->command(line) && !line.starts_with("\""))
        {
          log(":so command failed line") << i + 1 << line;
          result = false;
        }
      }

      i++;
    }
  }

  return result;
};
}; // namespace wex

wex::ex::commands_t wex::ex::commands_ex()
{
  // These are the commands without address specifier,
  // for these commands see address.cpp and addressrange.cpp.
  return {
    {"^:ab(breviate)?\\b",
     [&](const std::string& command)
     {
       return handle_container<std::string, macros::strings_map_t>(
         "Abbreviations",
         command,
         &m_macros.get_abbreviations(),
         [=, this](const std::string& name, const std::string& value)
         {
           m_macros.set_abbreviation(name, value);
           return true;
         });
     }},
    {"^:ar(gs)?\\b",
     [&](const std::string& command)
     {
       std::stringstream text;
       for (size_t i = 1; i < wxTheApp->argv.GetArguments().size(); i++)
       {
         text << wxTheApp->argv.GetArguments()[i] << "\n";
       }
       if (!text.str().empty())
       {
         show_dialog("ar", text.str());
       }
       return true;
     }},
    {"^:chd(ir)?\\b|:cd\\b",
     [&](const std::string& command)
     {
       if (!command.contains(" "))
       {
         wex::path::current(path(wxGetHomeDir().ToStdString()));
       }
       else
       {
         wex::path::current(path(wex::find_first_of(
           boost::algorithm::trim_right_copy(command),
           " ")));
       }

       return true;
     }},
    {"^:close\\b",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_CLOSE) return true;
     }},
    {"^:de\\b",
     [&](const std::string& command)
     {
       m_frame->debug_exe(wex::find_first_of(command, " "), get_stc());
       return true;
     }},
    {"^:e(dit)?\\b",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_OPEN) return true;
     }},
    {"^:f\\b",
     [&](const std::string& command)
     {
       std::stringstream text;
       text << get_stc()->path().filename() << " line "
            << get_stc()->get_current_line() + 1 << " of "
            << get_stc()->get_line_count() << " --"
            << 100 * (get_stc()->get_current_line() + 1) /
                 get_stc()->get_line_count()
            << "--%"
            << " level " << get_stc()->get_fold_level();
       m_frame->show_ex_message(text.str());
       return true;
     }},
    {"^:grep\\b",
     [&](const std::string& command)
     {
       POST_COMMAND(ID_TOOL_REPORT_FIND) return true;
     }},
    {"^:gt\\b",
     [&](const std::string& command)
     {
       return get_stc()->link_open();
     }},
    {"^:help\\b",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_HELP) return true;
     }},
    {"^:map\\b",
     [&](const std::string& command)
     {
       switch (get_command_arg(command))
       {
         case wex::command_arg_t::INT:
           return handle_container<int, wex::macros::keys_map_t>(
             "Map",
             command,
             nullptr,
             [=, this](const std::string& name, const std::string& value)
             {
               m_macros.set_key_map(name, value);
               return true;
             });

         case wex::command_arg_t::NONE:
           show_dialog(
             "Maps",
             "[String map]\n" +
               report_container<std::string, macros::strings_map_t>(
                 m_macros.get_map()) +
               "[Key map]\n" +
               report_container<int, wex::macros::keys_map_t>(
                 m_macros.get_keys_map()) +
               "[Alt key map]\n" +
               report_container<int, wex::macros::keys_map_t>(
                 m_macros.get_keys_map(macros::key_t::ALT)) +
               "[Control key map]\n" +
               report_container<int, wex::macros::keys_map_t>(
                 m_macros.get_keys_map(macros::key_t::CONTROL)),
             lexer_props().scintilla_lexer());
           return true;

         case wex::command_arg_t::OTHER:
           return handle_container<std::string, macros::strings_map_t>(
             "Map",
             command,
             nullptr,
             [=, this](const std::string& name, const std::string& value)
             {
               m_macros.set_map(name, value);
               return true;
             });
       }
       return false;
     }},
    {"^:marks\\b",
     [&](const std::string& command)
     {
       const lexer_props           l;
       std::map<char, std::string> sorted;
       for (const auto& marker : m_marker_identifiers)
       {
         sorted[marker.first] = l.make_key(
           std::string(1, marker.first),
           std::to_string(marker_line(marker.first) + 1) + ":" +
             std::to_string(m_marker_columns[marker.first]));
       }
       std::string text("[line:col]\n");
       for (const auto& it : sorted)
       {
         text += it.second;
       }
       show_dialog("Markers", text, l.scintilla_lexer());
       return true;
     }},
    {"^:new\\b",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_NEW) return true;
     }},
    {"^:print\\b",
     [&](const std::string& command)
     {
       get_stc()->print();
       return true;
     }},
    {"^:pwd\\b",
     [&](const std::string& command)
     {
       wex::log::status(wex::path::current().string());
       return true;
     }},
    {"^:q(uit)?!?\\b",
     [&](const std::string& command)
     {
       POST_CLOSE(wxEVT_CLOSE_WINDOW, !command.contains("!"))
       return true;
     }},
    {"^:reg\\b",
     [&](const std::string& command)
     {
       const lexer_props l;
       const auto&       regs(m_macros.get_registers());
       const auto        output(
         std::accumulate(
           regs.begin(),
           regs.end(),
           l.make_section("Named buffers")) +
         l.make_section("Filename buffer") +
         l.make_key("%", m_command.get_stc()->path().filename()));
       show_dialog("Registers", output, l.scintilla_lexer());
       return true;
     }},
    {"^:sed\\b",
     [&](const std::string& command)
     {
       POST_COMMAND(ID_TOOL_REPLACE) return true;
     }},
    {"^:set?\\b",
     [&](const std::string& command)
     {
       return command_set(command);
     }},
    {"^:so(urce)?\\b",
     [&](const std::string& cmd)
     {
       return source(this, cmd);
     }},
    {"^:syntax\\b",
     [&](const std::string& command)
     {
       if (command.ends_with("on"))
       {
         wex::lexers::get()->restore_theme();
         get_stc()->get_lexer().set(
           path_lexer(get_stc()->path()).lexer(),
           true); // allow folding
       }
       else if (command.ends_with("off"))
       {
         get_stc()->get_lexer().clear();
         wex::lexers::get()->clear_theme();
       }
       else
       {
         return false;
       }
       m_frame->statustext(wex::lexers::get()->theme(), "PaneTheme");
       return true;
     }},
    {"^:tag?\\b",
     [&](const std::string& command)
     {
       ctags::find(wex::find_first_of(command, " "));
       return true;
     }},
    {"^:una(bbrev)?\\b",
     [&](const std::string& command)
     {
       if (command.contains(" "))
       {
         m_macros.set_abbreviation(find_after(command, " "), "");
       }
       return true;
     }},
    {"^:unm(ap)?\\b",
     [&](const std::string& command)
     {
       if (command.contains(" "))
       {
         switch (get_command_arg(command))
         {
           case wex::command_arg_t::INT:
             m_macros.set_key_map(find_after(command, " "), "");
             break;
           case wex::command_arg_t::NONE:
             break;
           case wex::command_arg_t::OTHER:
             m_macros.set_map(find_after(command, " "), "");
             break;
         }
       }
       return true;
     }},
    {"^:ve(rsion)?\\b",
     [&](const std::string& command)
     {
       show_dialog("Version", wex::get_version_info().get());
       return true;
     }}};
}

bool wex::ex::command_handle(const std::string& command) const
{
  const auto& it = std::ranges::find_if(
    m_commands,
    [command](auto const& e)
    {
      return std::regex_search(command, std::regex(e.first));
    });

  return it != m_commands.end() && it->second(command);
}
