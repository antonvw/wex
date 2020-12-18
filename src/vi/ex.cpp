////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wex::ex
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ex.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "eval.h"
#include <wex/address.h>
#include <wex/addressrange.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/ctags.h>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/lexer-props.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/managed-frame.h>
#include <wex/statusbar.h>
#include <wex/stc-entry-dialog.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/type-to-value.h>
#include <wex/version.h>

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
    if (const auto post(wex::after(command, ' ')); post == command)
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

    wex::path path(wex::firstof(cmd, " "));

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
      wex::tokenizer tkz(*buffer, "\r\n");
      int            i = 0;

      while (tkz.has_more_tokens())
      {
        if (const std::string line(tkz.get_next_token()); !line.empty())
        {
          if (line == cmd)
          {
            log("recursive line") << i + 1 << line;
            return false;
          }

          if (
            line.starts_with(":a") || line.starts_with(":i") ||
            line.starts_with(":c"))
          {
            if (!ex->command(line + tkz.last_delimiter()))
            {
              log::trace("command insert failed line") << i + 1 << line;
              result = false;
            }
          }
          else if (!ex->command(line))
          {
            log::trace("command failed line") << i + 1 << line;
            result = false;
          }
        }

        i++;
      }
    }

    return result;
  };
}; // namespace wex

enum class wex::ex::address_t
{
  NONE,
  ONE,
  RANGE,
};

wex::macros wex::ex::m_macros;

wex::ex::ex(wex::stc* stc)
  : m_command(ex_command(stc))
  , m_frame(dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow()))
  , m_commands{{":ab",
                [&](const std::string& command) {
                  return handle_container<std::string, macros::strings_map_t>(
                    "Abbreviations",
                    command,
                    &m_macros.get_abbreviations(),
                    [=, this](const std::string& name, const std::string& value) {
                      m_macros.set_abbreviation(name, value);
                      return true;
                    });
                }},
               {":ar",
                [&](const std::string& command) {
                  std::stringstream text;
                  for (size_t i = 1; i < wxTheApp->argv.GetArguments().size();
                       i++)
                  {
                    text << wxTheApp->argv.GetArguments()[i] << "\n";
                  }
                  if (!text.str().empty())
                    show_dialog("ar", text.str());
                  return true;
                }},
               {":chd",
                [&](const std::string& command) {
                  if (command.find(" ") == std::string::npos)
                    return true;
                  wex::path::current(wex::firstof(command, " "));
                  return true;
                }},
               {":close",
                [&](const std::string& command) {
                  POST_COMMAND(wxID_CLOSE) return true;
                }},
               {":de",
                [&](const std::string& command) {
                  m_frame->get_debug()->execute(
                    wex::firstof(command, " "),
                    get_stc());
                  return true;
                }},
               {":e",
                [&](const std::string& command) {
                  POST_COMMAND(wxID_OPEN) return true;
                }},
               {":f",
                [&](const std::string& command) {
                  std::stringstream text;
                  text << get_stc()->get_filename().fullname() << " line "
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
                [&](const std::string& command) {
                  POST_COMMAND(ID_TOOL_REPORT_FIND) return true;
                }},
               {":gt",
                [&](const std::string& command) {
                  return get_stc()->link_open();
                }},
               {":help",
                [&](const std::string& command) {
                  POST_COMMAND(wxID_HELP) return true;
                }},
               {":map",
                [&](const std::string& command) {
                  switch (get_command_arg(command))
                  {
                    case wex::command_arg_t::INT:
                      return handle_container<int, wex::macros::keys_map_t>(
                        "Map",
                        command,
                        nullptr,
                        [=, this](const std::string& name, const std::string& value) {
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
                      return handle_container<
                        std::string,
                        macros::strings_map_t>(
                        "Map",
                        command,
                        nullptr,
                        [=, this](const std::string& name, const std::string& value) {
                          m_macros.set_map(name, value);
                          return true;
                        });
                  }
                  return false;
                }},
               {":new",
                [&](const std::string& command) {
                  POST_COMMAND(wxID_NEW) return true;
                }},
               {":print",
                [&](const std::string& command) {
                  get_stc()->print(command.find(" ") == std::string::npos);
                  return true;
                }},
               {":pwd",
                [&](const std::string& command) {
                  wex::log::status(wex::path::current());
                  return true;
                }},
               {":q!",
                [&](const std::string& command) {
                  POST_CLOSE(wxEVT_CLOSE_WINDOW, false) return true;
                }},
               {":q",
                [&](const std::string& command) {
                  POST_CLOSE(wxEVT_CLOSE_WINDOW, true) return true;
                }},
               {":reg",
                [&](const std::string& command) {
                  const lexer_props l;
                  std::string       output(l.make_section("Named buffers"));
                  for (const auto& it : m_macros.get_registers())
                  {
                    output += it;
                  }
                  output += l.make_section("Filename buffer");
                  output += l.make_key(
                    "%",
                    get_command().get_stc()->get_filename().fullname());
                  show_dialog("Registers", output, l.scintilla_lexer());
                  return true;
                }},
               {":sed",
                [&](const std::string& command) {
                  POST_COMMAND(ID_TOOL_REPLACE) return true;
                }},
               {":set",
                [&](const std::string& command) {
                  return command_set(command);
                }},
               {":so",
                [&](const std::string& cmd) {
                  return source(this, cmd);
                }},
               {":syntax",
                [&](const std::string& command) {
                  if (command.ends_with("on"))
                  {
                    wex::lexers::get()->restore_theme();
                    get_stc()->get_lexer().set(
                      get_stc()->get_filename().lexer().display_lexer(),
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
                [&](const std::string& command) {
                  ctags::find(wex::firstof(command, " "));
                  return true;
                }},
               {":una",
                [&](const std::string& command) {
                  if (wex::tokenizer tkz(command); tkz.count_tokens() >= 1)
                  {
                    tkz.get_next_token(); // skip :una
                    m_macros.set_abbreviation(tkz.get_next_token(), "");
                  }
                  return true;
                }},
               {":unm",
                [&](const std::string& command) {
                  if (wex::tokenizer tkz(command); tkz.count_tokens() >= 1)
                  {
                    tkz.get_next_token(); // skip :unm
                    switch (get_command_arg(command))
                    {
                      case wex::command_arg_t::INT:
                        m_macros.set_key_map(tkz.get_next_token(), "");
                        break;
                      case wex::command_arg_t::NONE:
                        break;
                      case wex::command_arg_t::OTHER:
                        m_macros.set_map(tkz.get_next_token(), "");
                        break;
                    }
                  }
                  return true;
                }},
               {":ve",
                [&](const std::string& command) {
                  show_dialog("Version", wex::get_version_info().get());
                  return true;
                }},
               {":x",
                [&](const std::string& command) {
                  if (command != ":x")
                    return false;
                  POST_COMMAND(wxID_SAVE)
                  POST_CLOSE(wxEVT_CLOSE_WINDOW, true)
                  return true;
                }}}
  , m_ctags(new wex::ctags(this))
  , m_auto_write(config(_("stc.Auto write")).get(false))
{
  assert(m_frame != nullptr);
  reset_search_flags();
}

wex::ex::~ex()
{
  delete m_ctags;
}

bool wex::ex::address_parse(
  std::string& text,
  std::string& range,
  std::string& cmd,
  address_t&   type)
{
  if (text.compare(0, 5, "'<,'>") == 0)
  {
    if (get_stc()->get_selected_text().empty())
    {
      return false;
    }

    type  = address_t::RANGE;
    range = "'<,'>";
    cmd   = text.substr(5);
    text  = text.substr(6);
  }
  else
  {
    marker_and_register_expansion(this, text);

    // Addressing in ex.
    const std::string addr(
      // (1) . (2) $ (3) decimal number, + or - (7)
      "[\\.\\$0-9\\+\\-]+|"
      // (4) marker
      "'[a-z]|"
      // (5) (6) regex find, non-greedy!
      "[\\?/].*?[\\?/]");

    // Command Descriptions in ex.
    // 1addr commands
    const auto& cmds_1addr(address(this).regex_commands());

    // 2addr commands
    const auto& cmds_2addr(addressrange(this).regex_commands());

    if (std::vector<std::string> v;
        // 2addr % range
        match("^%" + cmds_2addr, text, v) == 2 ||
        // 1addr (or none)
        match("^(" + addr + ")?" + cmds_1addr, text, v) == 3 ||
        // 2addr
        match("^(" + addr + ")?(," + addr + ")?" + cmds_2addr, text, v) == 4)
    {
      switch (v.size())
      {
        case 2:
          type  = address_t::RANGE;
          range = "%";
          cmd   = v[0];
          text  = v[1];
          break;

        case 3:
          type  = address_t::ONE;
          range = v[0];
          cmd   = (v[1] == "mark" ? "k" : v[1]);
          text  = trim(v[2], skip_t().set(TRIM_LEFT));
          break;

        case 4:
          type  = address_t::RANGE;
          range = v[0] + v[1];

          if (v[2].substr(0, 2) == "co")
          {
            cmd = "t";
          }
          else if (v[2].substr(0, 2) == "nu")
          {
            cmd = "#";
          }
          else
          {
            cmd = v[2];
          }

          text = v[3];
          break;

        default:
          assert(0);
          break;
      }
    }
    else
    {
      type = address_t::NONE;
      const auto line(address(this, text).get_line());

      if (line > 0)
        data::stc(get_stc()).control(data::control().line(line)).inject();

      return line > 0;
    }

    if (range.empty() && cmd != '!')
    {
      range = (cmd == "g" || cmd == 'v' || cmd == 'w' ? "%" : ".");
    }
  }

  return true;
}

bool wex::ex::auto_write()
{
  if (!m_auto_write || !get_stc()->IsModified())
  {
    return true;
  }

  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE);

  wxPostEvent(wxTheApp->GetTopWindow(), event);

  return true;
}

int wex::ex::calculator(const std::string& text)
{
  const auto& [val, err] = evaluator().eval(this, text);

  if (!err.empty())
  {
    show_dialog("Calculate Error", err);
  }

  return val;
}

bool wex::ex::command(const std::string& cmd)
{
  auto command(cmd);

  if (!m_is_active || command.empty() || command.front() != ':')
    return false;

  log::trace("ex command") << cmd;

  const auto& it = m_macros.get_map().find(command);
  command        = (it != m_macros.get_map().end() ? it->second : command);

  if (m_frame->exec_ex_command(m_command.set(command)))
  {
    m_macros.record(command);
    m_command.clear();
    return auto_write();
  }
  else if (command == ":" || command == ":'<,'>" || command == ":!")
  {
    return m_frame->show_ex_command(this, command);
  }
  else if (!command_handle(command) && !command_address(command.substr(1)))
  {
    m_command.clear();
    return false;
  }
  else
  {
    m_macros.record(command);
  }

  m_command.clear();

  return auto_write();
}

bool wex::ex::command_address(const std::string& command)
{
  std::string range, cmd, rest(command);
  address_t   type;

  if (!address_parse(rest, range, cmd, type))
  {
    return false;
  }

  try
  {
    switch (type)
    {
      case address_t::NONE:
        break;

      case address_t::ONE:
        if (!address(this, range).parse(cmd, rest))
        {
          return false;
        }
        break;

      case address_t::RANGE:
        if (info_message_t im; !addressrange(this, range).parse(cmd, rest, im))
        {
          return false;
        }
        else if (im != info_message_t::NONE)
        {
          info_message(register_text(), im);
        }
        break;

      default:
        assert(0);
    }
  }
  catch (std::exception& e)
  {
    log(e) << command;
    return false;
  }

  return true;
}

bool wex::ex::command_handle(const std::string& command) const
{
  const auto& it = std::find_if(
    m_commands.begin(),
    m_commands.end(),
    [command](auto const& e) {
      return e.first == command.substr(0, e.first.size());
    });

  return it != m_commands.end() && it->second(command);
}

bool wex::ex::command_set(const std::string& command)
{
  const bool modeline = command.back() == '*';

  wex::cmdline cmdline(
    // switches
    {{{"ac", _("Auto complete")}, nullptr},
     {{"ai", "ex-set.ai"},
      [&](bool on) {
        if (!modeline)
          config("stc.Auto indent").set(on ? (long)2 : (long)0);
      }},
     {{"aw", _("stc.Auto write")},
      [&](bool on) {
        m_auto_write = on;
      }},
     {{"eb", _("stc.Error bells")}, nullptr},
     {{"el", _("ex-set.el")},
      [&](bool on) {
        if (!modeline)
          config(_("stc.Edge line"))
            .set(on ? (long)wxSTC_EDGE_LINE : (long)wxSTC_EDGE_NONE);
        else
          get_stc()->SetEdgeMode(wxSTC_EDGE_LINE);
      }},
     {{"ic", "ex-set.ignorecase"},
      [&](bool on) {
        if (!on)
          m_search_flags |= wxSTC_FIND_MATCHCASE;
        else
          m_search_flags &= ~wxSTC_FIND_MATCHCASE;
        wex::find_replace_data::get()->set_match_case(!on);
      }},
     {{"mw", "ex-set.matchwords"},
      [&](bool on) {
        if (on)
          m_search_flags |= wxSTC_FIND_WHOLEWORD;
        else
          m_search_flags &= ~wxSTC_FIND_WHOLEWORD;
        wex::find_replace_data::get()->set_match_word(on);
      }},
     {{"nu", _("stc.Line numbers")},
      [&](bool on) {
        if (modeline)
          get_stc()->show_line_numbers(on);
      }},
     {{"readonly", "ex-set.readonly"},
      [&](bool on) {
        get_stc()->SetReadOnly(on);
      }},
     {{"showmode", _("stc.Show mode")},
      [&](bool on) {
        m_frame->get_statusbar()->pane_show("PaneMode", on);
      }},
     {{"sm", _("stc.Show match")}, nullptr},
     {{"sws", "ex-set.showwhitespace"},
      [&](bool on) {
        if (!modeline)
        {
          config(_("stc.Whitespace visible"))
            .set(on ? wxSTC_WS_VISIBLEALWAYS : wxSTC_WS_INVISIBLE);
          config(_("stc.End of line")).set(on);
        }
        else
        {
          get_stc()->SetViewWhiteSpace(wxSTC_WS_VISIBLEALWAYS);
          get_stc()->SetViewEOL(true);
        }
      }},
     {{"ut", _("stc.Use tabs")},
      [&](bool on) {
        if (modeline)
          get_stc()->SetUseTabs(on);
      }},
     {{"wm", _("ex-set.wm")},
      [&](bool on) {
        if (!modeline)
          config(_("stc.Wrap line"))
            .set(on ? wxSTC_WRAP_CHAR : wxSTC_WRAP_NONE);
        else
          get_stc()->SetWrapMode(wxSTC_WRAP_CHAR);
      }},
     {{"ws", _("stc.Wrap scan"), "1"}, nullptr}},
    // options
    {{{"dir", "ex-set.dir", wex::path::current()},
      {cmdline::STRING,
       [&](const std::any& val) {
         wex::path::current(std::any_cast<std::string>(val));
       }}},
     {{"ec", _("stc.Edge column"), std::to_string(get_stc()->GetEdgeColumn())},
      {cmdline::INT,
       [&](const std::any& val) {
         if (!modeline)
           config(_("stc.Edge column")).set(std::any_cast<int>(val));
         else
           get_stc()->SetEdgeColumn(std::any_cast<int>(val));
       }}},
     {{"report",
       "stc.Reported lines",
       std::to_string(config("stc.Reported lines").get(5))},
      {cmdline::INT,
       [&](const std::any& val) {
         config("stc.Reported lines").set(std::any_cast<int>(val));
       }}},
     {{"sw", _("stc.Indent"), std::to_string(get_stc()->GetIndent())},
      {cmdline::INT,
       [&](const std::any& val) {
         if (!modeline)
           config(_("stc.Indent")).set(std::any_cast<int>(val));
         else
           get_stc()->SetIndent(std::any_cast<int>(val));
       }}},
     {{"sy", "ex-set.syntax"},
      {cmdline::STRING,
       [&](const std::any& val) {
         if (std::any_cast<std::string>(val) != "off")
           get_stc()->get_lexer().set(
             std::any_cast<std::string>(val),
             true); // allow folding
         else
           get_stc()->get_lexer().clear();
       }}},
     {{"ts", "stc.Tab width", std::to_string(get_stc()->GetTabWidth())},
      {cmdline::INT,
       [&](const std::any& val) {
         if (modeline)
           get_stc()->SetTabWidth(std::any_cast<int>(val));
       }}}},
    {},
    // no standard options
    false);

  bool          found;
  data::cmdline cmdl(command.substr(
    4,
    command.back() == '*' ? command.size() - 5 : std::string::npos));
  cmdl.save(!modeline);

  if ((found = cmdline.parse_set(cmdl)) && !modeline)
  {
    m_frame->on_command_item_dialog(
      wxID_PREFERENCES,
      wxCommandEvent(wxEVT_BUTTON, wxOK));
  }

  if (!cmdl.help().empty())
  {
    m_frame->output(cmdl.help());

    if (!modeline)
    {
      show_dialog("Options", cmdl.help(), lexer_props().scintilla_lexer());
    }
  }

  if (found)
  {
    log::trace(":set") << command;
  }

  return found;
}

void wex::ex::copy(const wex::ex* ex)
{
  m_marker_identifiers = ex->m_marker_identifiers;
  m_copy               = true; // no char numbers for a copy
}

void wex::ex::cut()
{
  const auto& sel(get_stc()->get_selected_text());

  yank('0');

  get_stc()->ReplaceSelection(wxEmptyString);

  set_registers_delete(sel);

  info_message(sel, wex::info_message_t::DEL);
}

template <typename S, typename T>
bool wex::ex::handle_container(
  const std::string&                                          kind,
  const std::string&                                          command,
  const T*                                                    container,
  std::function<bool(const std::string&, const std::string&)> cb)
{
  if (tokenizer tkz(command); tkz.count_tokens() >= 2)
  {
    tkz.get_next_token(); // skip
    const auto name(tkz.get_next_token());
    cb(name, tkz.get_string());
  }
  else if (container != nullptr)
  {
    show_dialog(
      kind,
      report_container<S, T>(*container),
      lexer_props().scintilla_lexer());
  }

  return true;
}

void wex::ex::info_message(const std::string& text, wex::info_message_t type)
  const
{
  if (const auto lines = get_number_of_lines(text);
      lines > config("stc.Reported lines").get(5))
  {
    wxString msg;

    switch (type)
    {
      case wex::info_message_t::ADD:
        msg = _("%d lines added");
        break;
      case wex::info_message_t::COPY:
        msg = _("%d lines copied");
        break;
      case wex::info_message_t::DEL:
        msg = _("%d fewer lines");
        break;
      case wex::info_message_t::MOVE:
        msg = _("%d lines moved");
        break;
      case wex::info_message_t::YANK:
        msg = _("%d lines yanked");
        break;
      default:
        return;
    }

    m_frame->show_ex_message(wxString::Format(msg, lines - 1));
  }
}

bool wex::ex::marker_add(char marker, int line)
{
  if (m_copy)
    return false;

  const wex::marker lm(wex::lexers::get()->get_marker(m_marker_symbol));

  if (!lm.is_ok())
  {
    wex::log("could not find marker symbol")
      << m_marker_symbol.number() << " in lexers";
    return false;
  }

  marker_delete(marker);

  int       id;
  const int lin = (line == -1 ? get_stc()->get_current_line() : line);

  if (lm.symbol() == wxSTC_MARK_CHARACTER)
  {
    if (const auto& it = m_marker_numbers.find(marker);
        it == m_marker_numbers.end())
    {
      // We have symbol:
      // 0: non-char ex marker
      // 1: change marker
      // 2: breakpoint marker
      // 3..: character markers (all markers in m_marker_identifiers)
      const auto marker_offset = 3;
      const auto marker_number = m_marker_identifiers.size() + marker_offset;

      get_stc()->MarkerDefine(
        marker_number,
        wxSTC_MARK_CHARACTER + marker,
        wxColour(lm.foreground_colour()),
        wxColour(lm.background_colour()));

      id                       = get_stc()->MarkerAdd(lin, marker_number);
      m_marker_numbers[marker] = marker_number;
    }
    else
    {
      id = get_stc()->MarkerAdd(lin, it->second);
    }
  }
  else
  {
    id = get_stc()->MarkerAdd(lin, m_marker_symbol.number());
  }

  if (id == -1)
  {
    log("could not add marker") << marker << "to line:" << lin;
    return false;
  }

  m_marker_identifiers[marker] = id;

  return true;
}

bool wex::ex::marker_delete(char marker)
{
  if (const auto& it = m_marker_identifiers.find(marker);
      it != m_marker_identifiers.end())
  {
    get_stc()->MarkerDeleteHandle(it->second);
    m_marker_identifiers.erase(it);
    return true;
  }

  return false;
}

bool wex::ex::marker_goto(char marker)
{
  if (const auto line = marker_line(marker); line != -1)
  {
    data::stc(get_stc()).control(data::control().line(line + 1)).inject();
    return true;
  }

  return false;
}

int wex::ex::marker_line(char marker) const
{
  if (marker == '<')
  {
    if (!get_stc()->get_selected_text().empty())
    {
      return get_stc()->LineFromPosition(get_stc()->GetSelectionStart());
    }
  }
  else if (marker == '>')
  {
    if (!get_stc()->get_selected_text().empty())
    {
      return get_stc()->LineFromPosition(get_stc()->GetSelectionEnd());
    }
  }
  else
  {
    if (const auto& it = m_marker_identifiers.find(marker);
        it != m_marker_identifiers.end())
    {
      if (const auto line = get_stc()->MarkerLineFromHandle(it->second);
          line == -1)
      {
        log::status("Handle for marker") << marker << "invalid";
      }
      else
      {
        return line;
      }
    }
    else
    {
      log::status("Undefined marker") << marker;
    }
  }

  if (config(_("stc.Error bells")).get(true))
  {
    wxBell();
  }

  return -1;
}

void wex::ex::print(const std::string& text)
{
  show_dialog("Print", text);
}

const std::string wex::ex::register_insert() const
{
  return m_macros.get_register('.');
}

const std::string wex::ex::register_text() const
{
  return m_register ? m_macros.get_register(m_register) :
                      m_macros.get_register('0');
}

template <typename S, typename T>
std::string wex::ex::report_container(const T& t) const
{
  const wex::lexer_props l;
  std::string            output;

  for (const auto& it : t)
  {
    output += l.make_key(type_to_value<S>(it.first).get_string(), it.second);
  }

  return output;
}

void wex::ex::reset_search_flags()
{
  m_search_flags =
    ((find_replace_data::get()->match_case() ? wxSTC_FIND_MATCHCASE : 0) |
     wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX);
}

void wex::ex::set_registers_delete(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }

  for (int i = 9; i >= 2; i--)
  {
    if (const auto value(m_macros.get_register(char(48 + i - 1)));
        !value.empty())
    {
      m_macros.set_register(char(48 + i), value);
    }
  }

  m_macros.set_register('1', value);
}

void wex::ex::set_register_insert(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }

  m_macros.set_register('.', value);
}

void wex::ex::set_register_yank(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }

  m_macros.set_register('0', value);
}

void wex::ex::show_dialog(
  const std::string& title,
  const std::string& text,
  const std::string& lexer)
{
  if (m_dialog == nullptr)
  {
    m_dialog = new stc_entry_dialog(
      text,
      std::string(),
      data::window().button(wxOK).title(title).size({450, 450}));
  }
  else
  {
    if (title == "Print")
    {
      if (title != m_dialog->GetTitle())
      {
        m_dialog->get_stc()->set_text(text);
      }
      else
      {
        m_dialog->get_stc()->AppendText(text);
        m_dialog->get_stc()->DocumentEnd();
      }
    }
    else
    {
      m_dialog->get_stc()->set_text(text);
    }

    m_dialog->SetTitle(title);
  }

  m_dialog->get_stc()->get_lexer().set(
    !lexer.empty() ? wex::lexer(lexer) : get_stc()->get_lexer());
  m_dialog->Show();
}

bool wex::ex::yank(char name) const
{
  const auto& range(get_stc()->get_selected_text());

  if (range.empty())
  {
    return false;
  }
  else if (register_name())
  {
    m_macros.set_register(register_name(), range);
  }
  else if (name != '0')
  {
    m_macros.set_register(name, range);
  }
  else
  {
    set_register_yank(range);
  }

  info_message(range, wex::info_message_t::YANK);

  return true;
}
