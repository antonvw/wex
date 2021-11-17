////////////////////////////////////////////////////////////////////////////////
// Name:      comands-ex.cpp
// Purpose:   Implementation of class wex::ex::commands_ex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wex/core/version.h>
#include <wex/factory/lexers.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/vi/ctags.h>
#include <wex/vi/ex.h>
#include <wex/vi/macros.h>
#include <wx/app.h>

#include <sstream>

#define POST_CLOSE(ID, VETO)                      \
  {                                               \
    wxCloseEvent event(ID);                       \
    event.SetCanVeto(VETO);                       \
    wxPostEvent(wxTheApp->GetTopWindow(), event); \
  };

#define POST_COMMAND(ID)                                      \
  {                                                           \
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID);    \
                                                              \
    if (command.find(" ") != std::string::npos)               \
    {                                                         \
      event.SetString(command.substr(command.find(" ") + 1)); \
    }                                                         \
                                                              \
    wxPostEvent(wxTheApp->GetTopWindow(), event);             \
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
  if (const auto& post(wex::after(command, ' ')); post == command)
  {
    return command_arg_t::NONE;
  }
  else
  {
    try
    {
      if (std::stoi(post) > 0)
      {
        return command_arg_t::INT;
      }
    }
    catch (std::exception&)
    {
      return command_arg_t::OTHER;
    }
  }

  return command_arg_t::OTHER;
}

bool source(ex* ex, const std::string& cmd)
{
  if (cmd.find(" ") == std::string::npos)
  {
    return false;
  }

  wex::path path(wex::first_of(cmd, " "));

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
      if (const std::string line(it); !line.empty())
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
  return {
    {":ab",
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
    {":ar",
     [&](const std::string& command)
     {
       std::stringstream text;
       for (size_t i = 1; i < wxTheApp->argv.GetArguments().size(); i++)
       {
         text << wxTheApp->argv.GetArguments()[i] << "\n";
       }
       if (!text.str().empty())
         show_dialog("ar", text.str());
       return true;
     }},
    {":chd",
     [&](const std::string& command)
     {
       if (command.find(" ") == std::string::npos)
         return true;
       wex::path::current(path(wex::first_of(command, " ")));
       return true;
     }},
    {":close",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_CLOSE) return true;
     }},
    {":de",
     [&](const std::string& command)
     {
       m_frame->debug_exe(wex::first_of(command, " "), get_stc());
       return true;
     }},
    {":e",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_OPEN) return true;
     }},
    {":f",
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
    {":grep",
     [&](const std::string& command)
     {
       POST_COMMAND(ID_TOOL_REPORT_FIND) return true;
     }},
    {":gt",
     [&](const std::string& command)
     {
       return get_stc()->link_open();
     }},
    {":help",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_HELP) return true;
     }},
    {":map",
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
                 m_macros.get_keys_map(macros::KEY_ALT)) +
               "[Control key map]\n" +
               report_container<int, wex::macros::keys_map_t>(
                 m_macros.get_keys_map(macros::KEY_CONTROL)),
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
    {":new",
     [&](const std::string& command)
     {
       POST_COMMAND(wxID_NEW) return true;
     }},
    {":print",
     [&](const std::string& command)
     {
       get_stc()->print(command.find(" ") == std::string::npos);
       return true;
     }},
    {":pwd",
     [&](const std::string& command)
     {
       wex::log::status(wex::path::current().string());
       return true;
     }},
    {":q!",
     [&](const std::string& command)
     {
       POST_CLOSE(wxEVT_CLOSE_WINDOW, false) return true;
     }},
    {":q",
     [&](const std::string& command)
     {
       POST_CLOSE(wxEVT_CLOSE_WINDOW, true) return true;
     }},
    {":reg",
     [&](const std::string& command)
     {
       const lexer_props l;
       std::string       output(l.make_section("Named buffers"));
       for (const auto& it : m_macros.get_registers())
       {
         output += it;
       }
       output += l.make_section("Filename buffer");
       output += l.make_key("%", get_command().get_stc()->path().filename());
       show_dialog("Registers", output, l.scintilla_lexer());
       return true;
     }},
    {":sed",
     [&](const std::string& command)
     {
       POST_COMMAND(ID_TOOL_REPLACE) return true;
     }},
    {":set",
     [&](const std::string& command)
     {
       return command_set(command);
     }},
    {":so",
     [&](const std::string& cmd)
     {
       return source(this, cmd);
     }},
    {":syntax",
     [&](const std::string& command)
     {
       if (command.ends_with("on"))
       {
         wex::lexers::get()->restore_theme();
         get_stc()->get_lexer().set(
           get_stc()->get_lexer().display_lexer(),
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
    {":ta",
     [&](const std::string& command)
     {
       ctags::find(wex::first_of(command, " "));
       return true;
     }},
    {":una",
     [&](const std::string& command)
     {
       if (command.find(" ") != std::string::npos)
       {
         m_macros.set_abbreviation(after(command, ' '), "");
       }
       return true;
     }},
    {":unm",
     [&](const std::string& command)
     {
       if (command.find(" ") != std::string::npos)
       {
         switch (get_command_arg(command))
         {
           case wex::command_arg_t::INT:
             m_macros.set_key_map(after(command, ' '), "");
             break;
           case wex::command_arg_t::NONE:
             break;
           case wex::command_arg_t::OTHER:
             m_macros.set_map(after(command, ' '), "");
             break;
         }
       }
       return true;
     }},
    {":ve",
     [&](const std::string& command)
     {
       show_dialog("Version", wex::get_version_info().get());
       return true;
     }},
    {":x",
     [&](const std::string& command)
     {
       if (command != ":x")
         return false;
       POST_COMMAND(wxID_SAVE)
       POST_CLOSE(wxEVT_CLOSE_WINDOW, true)
       return true;
     }}};
}

bool wex::ex::command_handle(const std::string& command) const
{
  const auto& it = std::find_if(
    m_commands.begin(),
    m_commands.end(),
    [command](auto const& e)
    {
      return e.first == command.substr(0, e.first.size());
    });

  return it != m_commands.end() && it->second(command);
}
