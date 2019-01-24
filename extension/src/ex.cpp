////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wex::ex
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ex.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/ex.h>
#include <wex/address.h>
#include <wex/addressrange.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/ctags.h>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/frd.h>
#include <wex/lexer-props.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/stcdlg.h>
#include <wex/tokenizer.h>
#include <wex/type-to-value.h>
#include <wex/util.h>
#include <wex/version.h>
#include <wex/vi-macros.h>
#include "eval.h"

#define POST_CLOSE( ID, VETO )                              \
{                                                           \
  wxCloseEvent event(ID);                                   \
  event.SetCanVeto(VETO);                                   \
  wxPostEvent(wxTheApp->GetTopWindow(), event);             \
};                                                          \

#define POST_COMMAND( ID )                                  \
{                                                           \
  wxCommandEvent event(                                     \
    wxEVT_COMMAND_MENU_SELECTED, ID);                       \
                                                            \
  if (command.find(" ") != std::string::npos)               \
  {                                                         \
    event.SetString(command.substr(command.find(" ") + 1)); \
  }                                                         \
                                                            \
  wxPostEvent(wxTheApp->GetTopWindow(), event);             \
};                                                          \

namespace wex
{
  enum class command_arg
  {
    INT,
    NONE,
    OTHER,
  };

  enum class info_message_t
  {
    ADD,
    COPY,
    DEL,
  };

  command_arg ParseCommandWithArg(const std::string& command)
  {
    if (const auto post(wex::after(command, ' ')); post == command)
    {
      return command_arg::NONE;
    }
    else if (atoi(post.c_str()) > 0)
    {
      return command_arg::INT;
    }
    else
    {
      return command_arg::OTHER;
    }
  }
};

wex::evaluator wex::ex::m_Evaluator;
wex::vi_macros wex::ex::m_Macros;

wex::ex::ex(wex::stc* stc)
  : m_Command(ex_command(stc))
  , m_CTags(new wex::ctags(this))
  , m_Frame(wxDynamicCast(wxTheApp->GetTopWindow(), managed_frame))
  , m_Commands {
    {":ab", [&](const std::string& command) {
      return HandleContainer<std::string, std::map<std::string, std::string>>(
        "Abbreviations", command, &m_Macros.get_abbreviations(),
        [=](const std::string& name, const std::string& value) {
          m_Macros.set_abbreviation(name, value);return true;});}},
    {":ar", [&](const std::string& command) {
      wxString text;
      for (size_t i = 1; i < wxTheApp->argv.GetArguments().size(); i++)
      {
        text << wxTheApp->argv.GetArguments()[i] << "\n";
      }
      if (!text.empty()) show_dialog("ar", text.ToStdString());
      return true;}},
    {":chd", [&](const std::string& command) {
      if (command.find(" ") == std::string::npos) return true;
      wex::path::current(wex::firstof(command, " ")); return true;}},
    {":close", [&](const std::string& command) {POST_COMMAND( wxID_CLOSE ) return true;}},
    {":de", [&](const std::string& command) {
      m_Frame->get_debug()->execute(wex::firstof(command, " "), m_Command.get_stc());
      return true;}},
    {":e", [&](const std::string& command) {POST_COMMAND( wxID_OPEN ) return true;}},
    {":f", [&](const std::string& command) {info_message(); return true;}},
    {":grep", [&](const std::string& command) {POST_COMMAND( ID_TOOL_REPORT_FIND ) return true;}},
    {":gt", [&](const std::string& command) {return m_Command.get_stc()->link_open();}},
    {":help", [&](const std::string& command) {POST_COMMAND( wxID_HELP ) return true;}},
    {":map", [&](const std::string& command) {
      switch (ParseCommandWithArg(command))
      {
        case wex::command_arg::INT:
          // TODO: at this moment you cannot set KEY_CONTROL
          return HandleContainer<int, wex::vi_macros_maptype>(
            "Map", command, nullptr,
            [=](const std::string& name, const std::string& value) {
              m_Macros.set_key_map(name, value);return true;}); 
        break;
        case wex::command_arg::NONE: show_dialog("Maps", 
            "[String map]\n" +
            ReportContainer<std::string, std::map<std::string, std::string>>(m_Macros.get_map()) +
            "[Key map]\n" +
            ReportContainer<int, wex::vi_macros_maptype>(m_Macros.get_keys_map()) +
            "[Alt key map]\n" +
            ReportContainer<int, wex::vi_macros_maptype>(m_Macros.get_keys_map(vi_macros::KEY_ALT)) +
            "[Control key map]\n" +
            ReportContainer<int, wex::vi_macros_maptype>(m_Macros.get_keys_map(vi_macros::KEY_CONTROL)), 
            true);
          return true;
        break;
        case wex::command_arg::OTHER:
          return HandleContainer<std::string, std::map<std::string, std::string>>(
            "Map", command, nullptr,
            [=](const std::string& name, const std::string& value) {
              m_Macros.set_map(name, value);return true;});
      }
      return false;}},
    {":new", [&](const std::string& command) {POST_COMMAND( wxID_NEW ) return true;}},
    {":print", [&](const std::string& command) {m_Command.get_stc()->print(command.find(" ") == std::string::npos); return true;}},
    {":pwd", [&](const std::string& command) {wex::log::status(wex::path::current()); return true;}},
    {":q!", [&](const std::string& command) {POST_CLOSE( wxEVT_CLOSE_WINDOW, false ) return true;}},
    {":q", [&](const std::string& command) {POST_CLOSE( wxEVT_CLOSE_WINDOW, true ) return true;}},
    {":reg", [&](const std::string& command) {
      show_dialog("Registers", m_Evaluator.info(this), true);
      return true;}},
    {":sed", [&](const std::string& command) {POST_COMMAND( ID_TOOL_REPLACE ) return true;}},
    {":set", [&](const std::string& command) {
        const bool save = command.back() != '*';
        std::string text(command.substr(4, 
          command.back() == '*' ? command.size() - 5: std::string::npos));
        // Convert arguments (add -- to each group, remove all =).
        // ts=120 ac ic sy=cpp -> --ts 120 --ac --ic --sy cpp
        std::regex re("[0-9a-z=]+");
        text = std::regex_replace(text, re, "--&", std::regex_constants::format_sed);
        std::replace(text.begin(), text.end(), '=', ' ');
        wex::cmdline cmdline(
          // switches
          {
           {{"ac", "autocomplete"}, [](bool on){
             config(_("Auto complete")).set(on);}},
           {{"ai", "autoindent"}, [](bool on){
             config("Auto indent").set(on ? (long)2: (long)0);}},
           {{"aw", "autowrite"}, [&](bool on){
             config(_("Auto write")).set(on);
             m_AutoWrite = on;}},
           {{"eb", "errorbells"}, [](bool on){
             config(_("Error bells")).set(on);}},
           {{"el", "edgeline"}, [&](bool on){
             m_Command.get_stc()->SetEdgeMode(
               on ? wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);
             config(_("Edge line")).set( 
               on ? (long)wxSTC_EDGE_LINE: (long)wxSTC_EDGE_NONE);}},
           {{"ic", "ignorecase"}, [&](bool on){
             if (!on) m_SearchFlags |= wxSTC_FIND_MATCHCASE;
             else     m_SearchFlags &= ~wxSTC_FIND_MATCHCASE;
             wex::find_replace_data::get()->set_match_case(!on);}},
           {{"mw", "matchwords"}, [&](bool on){
             if (on) m_SearchFlags |= wxSTC_FIND_WHOLEWORD;
             else    m_SearchFlags &= ~wxSTC_FIND_WHOLEWORD;
             wex::find_replace_data::get()->set_match_word(on);}},
           {{"nu", "number"}, [&](bool on){
             m_Command.get_stc()->show_line_numbers(on);
             config(_("Line numbers")).set(on);}},
           {{"readonly", "readonly"}, [&](bool on){
             m_Command.get_stc()->SetReadOnly(on);}},
           {{"showmode", "showmode"}, [&](bool on){
             ((wex::statusbar *)m_Frame->GetStatusBar())->show_field("PaneMode", on);
             config(_("Show mode")).set(on);}},
           {{"sm", "showmatch"}, [&](bool on){
             config(_("Show match")).set(on);}},
           {{"sws", "showwhitespace"}, [&](bool on){
             m_Command.get_stc()->SetViewEOL(on);
             m_Command.get_stc()->SetViewWhiteSpace(on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);
             config(_("Whitespace")).set(on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);}},
           {{"ut", "usetabs"}, [&](bool on){
             m_Command.get_stc()->SetUseTabs(on);
             config(_("Use tabs")).set(on);}},
           {{"wm", "wrapmargin"}, [&](bool on){
             m_Command.get_stc()->SetWrapMode(on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);
             config(_("Wrap line")).set(on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);}},
           {{"ws", "wrapscan", "1"}, [&](bool on){
             config(_("Wrap scan")).set(on);}}
          },

          // options
          {
           {{"dir", "dir"}, {cmdline::STRING, [&](const std::any& val) {
             wex::path::current(std::any_cast<std::string>(val));}}},
           {{"ec", "edgecolumn", "80"}, {cmdline::INT, [&](const std::any& val) {
             m_Command.get_stc()->SetEdgeColumn(std::any_cast<int>(val));
             config(_("Edge column")).set(std::any_cast<int>(val));}}},
           {{"report", "report", "5"}, {cmdline::INT, [&](const std::any& val) {
             config("Reported lines").set(std::any_cast<int>(val));}}},
           {{"sw", "shiftwidth", "8"}, {cmdline::INT, [&](const std::any& val) {
             m_Command.get_stc()->SetIndent(std::any_cast<int>(val));
             config(_("Indent")).set(std::any_cast<int>(val));}}},
           {{"sy", "syntax (lexer or 'off')"}, {cmdline::STRING, [&](const std::any& val) {
             if (std::any_cast<std::string>(val) != "off") 
               m_Command.get_stc()->get_lexer().set(std::any_cast<std::string>(val), true); // allow folding
             else              
               m_Command.get_stc()->get_lexer().reset();}}},
           {{"ts", "tabstop","8"}, {cmdline::INT, [&](const std::any& val) {
             m_Command.get_stc()->SetTabWidth(std::any_cast<int>(val));
             config(_("Tab width")).set(std::any_cast<int>(val));}}}
          }
        );

      if (command.find(" ") == std::string::npos)
      {
        cmdline.show_options(wex::window_data().size({200, 450}));
      }
      else
      {
        cmdline.parse(command.substr(0, 4) + text, save);
      }
      return true;}},
    {":so", [&](const std::string& command) {
      if (command.find(" ") == std::string::npos) return false;
      wex::path path(wex::firstof(command, " "));
      if (path.is_relative())
      {
        path.make_absolute();
      }
      std::ifstream ifs(path.data());
      if (!ifs.is_open()) return false;
      int i = 0;
      for (std::string line; std::getline(ifs, line); )
      {
        if (!line.empty())
        {
          if (line == command)
          {
            log::verbose("recursive line") << i + 1;
            return false;
          }
          else if (!ex::command(line))
          {
            log::verbose("command error line") << i + 1;
            return false;
          }
        }
        i++;
      }
      return true;}},
    {":syntax", [&](const std::string& command) {
      if (wxString(command).EndsWith("on"))
      {
        wex::lexers::get()->restore_theme();
        m_Command.get_stc()->get_lexer().set(m_Command.get_stc()->get_filename().lexer().display_lexer(), true); // allow folding
      }
      else if (wxString(command).EndsWith("off"))
      {
        m_Command.get_stc()->get_lexer().reset();
        wex::lexers::get()->reset_theme();
      }
      else
      {
        return false;
      }
      m_Frame->statustext(wex::lexers::get()->theme(), "PaneTheme");
      return true;}},
    {":ta", [&](const std::string& command) {
      m_CTags->find(wex::firstof(command, " "));
      return true;}},
    {":una", [&](const std::string& command) {
      if (wex::tokenizer tkz(command); tkz.count_tokens() >= 1)
      {
        tkz.get_next_token(); // skip :una
        m_Macros.set_abbreviation(tkz.get_next_token(), "");
      }
      return true;}},
    {":unm", [&](const std::string& command) {
      if (wex::tokenizer tkz(command); tkz.count_tokens() >= 1)
      {
        tkz.get_next_token(); // skip :unm
        switch (ParseCommandWithArg(command))
        {
          case wex::command_arg::INT: m_Macros.set_key_map(tkz.get_next_token(), ""); break; 
          case wex::command_arg::NONE: break;
          case wex::command_arg::OTHER: m_Macros.set_map(tkz.get_next_token(), ""); break;
        }
      }
      return true;}},
    {":ve", [&](const std::string& command) {show_dialog("Version", 
      wex::get_version_info().get()); return true;}},
    {":x", [&](const std::string& command) {
      if (command != ":x") return false;
      POST_COMMAND( wxID_SAVE )
      POST_CLOSE( wxEVT_CLOSE_WINDOW, true )
      return true;}}}
{
  assert(m_Frame != nullptr);
  reset_search_flags();
  m_AutoWrite = config(_("Auto write")).get(false);
}

wex::ex::~ex()
{
  delete m_CTags;
}
  
void wex::ex::add_text(const std::string& text)
{
  if (m_Register)
  {
    m_Macros.set_register(m_Register, text);
  }
  else
  {
    m_Command.get_stc()->AddTextRaw((const char *)text.c_str(), text.length());
  }

  info_message(text, wex::info_message_t::ADD);
}

bool wex::ex::auto_write()
{
  if (!m_AutoWrite || !m_Command.get_stc()->IsModified())
  {
    return true;
  }

  wxCommandEvent event(
    wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE);

  wxPostEvent(wxTheApp->GetTopWindow(), event);

  return true;
}

std::tuple<double, int> wex::ex::calculator(const std::string& text)
{
  const auto& [val, width, err] = m_Evaluator.Eval(this, text);

  if (!err.empty())
  {
    show_dialog("Error", text + "\n" + err);
  }

  return {val, width};
}

bool wex::ex::command(const std::string& cmd)
{
  auto command(cmd);

  if (!m_IsActive || command.empty() || command.front() != ':') return false;

  log::verbose("ex command") << cmd;

  const auto& it = m_Macros.get_map().find(command);
  command = (it != m_Macros.get_map().end() ? it->second: command);

  if (m_Frame->exec_ex_command(m_Command.command(cmd)))
  {
    m_Command.clear();
    return auto_write();
  }
  else if (command == ":" || command == ":'<,'>" || command == ":!")
  {
    return m_Frame->show_ex_command(this, command);
  }
  else if (
    !CommandHandle(command) &&
    !CommandAddress(command.substr(1)))
  {
    m_Command.clear();
    return false;
  }

  m_Macros.record(command);
  m_Command.clear();

  return auto_write();
}

bool wex::ex::CommandAddress(const std::string& command)
{
  auto rest(command);
  std::string range_str, cmd;
  bool addr1 = false; // single address

  if (rest.compare(0, 5, "'<,'>") == 0)
  {
    if (get_selected_text().empty())
    {
      return false;
    }

    range_str = "'<,'>";
    cmd = rest.substr(5);
    rest = rest.substr(6);
  }
  else
  { 
    const std::string addr("[0-9\\.\\$\\+\\-]+"); // addr (normal)
    const std::string addrs("[\\?/].*?[\\?/]"); // addr search, non-greedy!
    const std::string addrm("'[a-z]"); // addr using marker
    const std::string cmd_group1("([aikrz=]|pu)(.*)"); // 1 addr command
    const std::string cmd_group2("([cdgjmpsStvywy<>\\!&~])(.*)"); // 2 addr command
    
    if (std::vector <std::string> v;
      // a % address range
      match("^%" + cmd_group2, rest, v) == 2 ||
      // addr2 search
      match("^(" + addrs + ")(," + addrs + ")" + cmd_group2, rest, v) == 4 ||
      // addr1 search
      match("^(" + addrs + ")" + cmd_group1, rest, v) == 3 ||
      // addr2 markers
      match("^(" + addrm + ")(," + addrm + ")" + cmd_group2, rest, v) == 4 ||
      match("^(" + addr  + ")(," + addrm + ")" + cmd_group2, rest, v) == 4 ||
      match("^(" + addrm + ")(," +  addr + ")" + cmd_group2, rest, v) == 4 ||
      // addr1 marker
      match("^(" + addrm + ")" + cmd_group1, rest, v) == 3 ||
      // addr1
      match("^(" + addr + ")?" + cmd_group1, rest, v) == 3 ||
      // addr2
      match("^(" + addr + ")?(," + addr + ")?" + cmd_group2, rest, v) == 4)
    {
      switch (v.size())
      {
        case 2:
          range_str = "%";
          cmd = v[0];
          rest = v[1];
          break;
        case 3:
          addr1 = true;
          range_str = v[0];
          cmd = v[1];
          rest = skip_white_space(v[2], skip_t().set(SKIP_LEFT));
          break;
        case 4:
          range_str = v[0] + v[1];
          cmd = v[2];
          rest = v[3];
          break;
        default: assert(0); break;
      }

      if (!marker_and_register_expansion(this, range_str))
      {
        return false;
      }
    }
    else 
    {
      const auto line(address(this, rest).get_line());
      if (line > 0) stc_data(m_Command.get_stc()).control(control_data().line(line)).inject();
      return line > 0;
    }
    
    if (range_str.empty() && cmd != '!') 
    {
      range_str = (cmd == "g" || cmd == 'v' || cmd == 'w' ? "%": ".");
    }
  }
  
  if (addr1)
  {
    switch (const address addr(this, range_str); (int)cmd[0])
    {
    case 0: return false;
    case 'a': return addr.append(rest);
    case 'i': return addr.insert(rest);
    case 'k': return !rest.empty() ? addr.marker_add(rest[0]): false;
    case 'p': 
      if (cmd == "pu")
      { 
        return !rest.empty() ? addr.put(rest[0]): addr.put();
      }
      else
      {
        return false;
      }
      break;
    case 'r': return addr.read(rest);
    case 'z': return addr.adjust_window(rest);
    case '=': return addr.write_line_number();
    default:
      log::status("Unknown address command") << cmd;
      return false;
    }
  }
  else
  {
    switch (addressrange range(this, range_str); (int)cmd[0])
    {
    case 0: return false;
    case 'c': return range.change(rest);
    case 'd': return range.erase();
    case 'v':
    case 'g': return range.global(rest, cmd[0] == 'v');
    case 'j': return range.join();
    case 'm': return range.move(address(this, rest));
    case 'p': 
      if (m_Command.get_stc()->GetName() != "Print")
      {
        return range.print(rest);
      }
      else
      {
        return false;
      }
    case 's':
    case '&':
    case '~': return range.substitute(rest, cmd[0]);
    case 'S': return range.sort(rest);
    case 't': return range.copy(address(this, rest));
    case 'w': 
      if (!rest.empty())
      {
        return range.write(rest);
      }
      else
      {
        POST_COMMAND( wxID_SAVE )
        return true;
      }
      break;
    case 'y': return range.yank(rest.empty() ? '0': (char)rest[0]);
    case '>': return range.shift_right();
    case '<': return range.shift_left();
    case '!': return range.escape(rest);
    default:
      log::status("Unknown range command") << cmd;
      return false;
    }
  }
}

bool wex::ex::CommandHandle(const std::string& command) const
{
  const auto& it = std::find_if(m_Commands.begin(), m_Commands.end(), 
    [command](auto const& e) {return e.first == command.substr(0, e.first.size());});
  
  return it != m_Commands.end() && it->second(command);
}

void wex::ex::copy(const wex::ex* ex)
{
  m_MarkerIdentifiers = ex->m_MarkerIdentifiers;
  m_Copy = true; // no char numbers for a copy
}

void wex::ex::cut(bool show_message)
{
  const auto sel(get_selected_text());
  
  yank('0', false);

  m_Command.get_stc()->ReplaceSelection(wxEmptyString);
  
  set_registers_delete(sel);
  
  info_message(sel, wex::info_message_t::DEL);
}

const std::string wex::ex::register_insert() const
{
  return m_Macros.get_register('.');
}

const std::string wex::ex::register_text() const
{
  return m_Register ? 
    m_Macros.get_register(m_Register):
    m_Macros.get_register('0');
}
  
const std::string wex::ex::get_selected_text() const
{
  // This also supports rectangular text.
  if (m_Command.get_stc()->GetSelectedText().empty())
  {
    return std::string();
  }

  const wxCharBuffer b(m_Command.get_stc()->GetSelectedTextRaw());
  return std::string(b.data(), b.length() - 1);
}

template <typename S, typename T> 
bool wex::ex::HandleContainer(
  const std::string& kind,
  const std::string& command,
  const T * container,
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
    show_dialog(kind, ReportContainer<S, T>(*container), true);
  }

  return true;
}

void wex::ex::info_message() const
{
  m_Frame->show_ex_message(wxString::Format("%s line %d of %d --%d%%-- level %d", 
    m_Command.get_stc()->get_filename().fullname().c_str(), 
    m_Command.get_stc()->GetCurrentLine() + 1,
    m_Command.get_stc()->GetLineCount(),
    100 * (m_Command.get_stc()->GetCurrentLine() + 1)/ m_Command.get_stc()->GetLineCount(),
    (m_Command.get_stc()->GetFoldLevel(m_Command.get_stc()->GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK)
     - wxSTC_FOLDLEVELBASE).ToStdString());
}

void wex::ex::info_message(const std::string& text, wex::info_message_t type) const
{
  if (const auto lines = get_number_of_lines(text);
    lines >= config("Reported lines").get(5))
  {
    wxString msg;

    switch (type)
    {
      case wex::info_message_t::ADD: msg = _("%d lines added"); break;
      case wex::info_message_t::COPY: msg = _("%d lines yanked"); break;
      case wex::info_message_t::DEL: msg = _("%d fewer lines"); break;
    }

    m_Frame->show_ex_message(wxString::Format(msg, lines - 1).ToStdString());
  }
}

bool wex::ex::marker_add(char marker, int line)
{
  if (m_Copy) return false;

  const wex::marker lm(wex::lexers::get()->get_marker(m_MarkerSymbol));

  if (!lm.is_ok())
  {
    wex::log("could not find marker symbol") << m_MarkerSymbol.number() << " in lexers";
    return false;
  }
  
  marker_delete(marker);

  int id;
  const int lin = (line == -1 ? m_Command.get_stc()->GetCurrentLine(): line);

  if (lm.symbol() == wxSTC_MARK_CHARACTER)
  {
    if (const auto& it = m_MarkerNumbers.find(marker); it == m_MarkerNumbers.end())
    {
      // We have symbol:
      // 0: non-char ex marker
      // 1: change marker
      // 2: breakpoint marker
      // 3..: character markers (all markers in m_MarkerIdentifiers)
      const auto marker_offset = 3;
      const auto marker_number = m_MarkerIdentifiers.size() + marker_offset;

      m_Command.get_stc()->MarkerDefine(marker_number, 
        wxSTC_MARK_CHARACTER + marker, 
        wxString(lm.foreground_colour()), 
        wxString(lm.background_colour()));

      id = m_Command.get_stc()->MarkerAdd(lin, marker_number);
      m_MarkerNumbers[marker] = marker_number;
    }
    else
    {
      id = m_Command.get_stc()->MarkerAdd(lin, it->second);
    }
  }
  else
  {
    id = m_Command.get_stc()->MarkerAdd(lin, m_MarkerSymbol.number());
  }
    
  if (id == -1)
  {
    log("could not add marker") << marker  << "to line:" << lin;
    return false;  
  }
    
  m_MarkerIdentifiers[marker] = id;
  
  return true;
}  

bool wex::ex::marker_delete(char marker)
{
  if (const auto& it = m_MarkerIdentifiers.find(marker); it != m_MarkerIdentifiers.end())
  {
    m_Command.get_stc()->MarkerDeleteHandle(it->second);
    m_MarkerIdentifiers.erase(it);
    return true;
  }
  
  return false;
}

bool wex::ex::marker_goto(char marker)
{
  if (const auto line = marker_line(marker); line != -1)
  {
    stc_data(m_Command.get_stc()).control(control_data().line(line + 1)).inject();
    return true;
  }
  
  return false;
}

int wex::ex::marker_line(char marker) const
{
  if (marker == '<')
  {
    if (!get_selected_text().empty())
    {
      return m_Command.get_stc()->LineFromPosition(m_Command.get_stc()->GetSelectionStart());
    }
  }
  else if (marker == '>')
  {
    if (!get_selected_text().empty())
    {
      return m_Command.get_stc()->LineFromPosition(m_Command.get_stc()->GetSelectionEnd());
    }
  }
  else
  {
    if (const auto& it = m_MarkerIdentifiers.find(marker); it != m_MarkerIdentifiers.end())
    {
      if (const auto line = m_Command.get_stc()->MarkerLineFromHandle(it->second); line == -1)
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

  if (config(_("Error bells")).get(true))
  {
    wxBell();
  }

  return -1;
}

void wex::ex::print(const std::string& text)
{
  show_dialog("Print", text);
}
  
template <typename S, typename T>
std::string wex::ex::ReportContainer(const T & t) const
{
  const wex::lexer_props l;
  std::string output;

  for (const auto& it : t)
  {
    output += l.make_key(type_to_value<S>(it.first).get_string(), it.second);
  }

  return output;
}

void wex::ex::reset_search_flags()
{
  m_SearchFlags = ((find_replace_data::get()->match_case() ? 
    wxSTC_FIND_MATCHCASE: 0) | 
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
    if (const auto value(m_Macros.get_register(wxUniChar(48 + i - 1)));
      !value.empty())
    {
      m_Macros.set_register(wxUniChar(48 + i), value);
    }
  }
  
  m_Macros.set_register('1', value);
}
  
void wex::ex::set_register_insert(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  m_Macros.set_register('.', value);
}

void wex::ex::set_register_yank(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  m_Macros.set_register('0', value);
}

void wex::ex::show_dialog(
  const std::string& title, const std::string& text, bool prop_lexer)
{
  if (m_Dialog == nullptr)
  {
    m_Dialog = new stc_entry_dialog(
      text,
      std::string(),
      window_data().button(wxOK).title(title));
  }
  else
  {
    if (title == "Print")
    { 
      if (title != m_Dialog->GetTitle())
      {
        m_Dialog->get_stc()->set_text(text);
      }
      else
      {
        m_Dialog->get_stc()->AppendText(text);
        m_Dialog->get_stc()->DocumentEnd();
      }
    }
    else
    {
      m_Dialog->get_stc()->set_text(text);
    }
    
    m_Dialog->SetTitle(title);
  }
  
  m_Dialog->get_stc()->get_lexer().set(
    prop_lexer ? lexer_props(): m_Command.get_stc()->get_lexer());
  m_Dialog->Show();
}

bool wex::ex::yank(const char name, bool show_message) const
{
  const auto range(get_selected_text());
  
  if (range.empty())
  {
    return false;
  }
  else if (register_name())
  {
    m_Macros.set_register(register_name(), range);
  }
  else if (name != '0')
  {
    m_Macros.set_register(name, range);
  }
  else
  {
    set_register_yank(range);
  }

  info_message(range, wex::info_message_t::COPY);
  
  return true;
}
