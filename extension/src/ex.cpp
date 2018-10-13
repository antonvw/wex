////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wex::ex
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ex.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/ex.h>
#include <wx/extension/address.h>
#include <wx/extension/addressrange.h>
#include <wx/extension/cmdline.h>
#include <wx/extension/ctags.h>
#include <wx/extension/debug.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexer-props.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/type-to-value.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include <wx/extension/vi-macros.h>
#include <easylogging++.h>
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
  enum class commandarg
  {
    INT,
    NONE,
    OTHER,
  };

  enum class info_message
  {
    ADD,
    COPY,
    DEL,
  };
};

wex::commandarg ParseCommandWithArg(const std::string& command)
{
  if (const auto post(wex::after(command, ' ')); post == command)
  {
    return wex::commandarg::NONE;
  }
  else if (atoi(post.c_str()) > 0)
  {
    return wex::commandarg::INT;
  }
  else
  {
    return wex::commandarg::OTHER;
  }
}

wex::evaluator wex::ex::m_Evaluator;
wex::stc_entry_dialog* wex::ex::m_Dialog = nullptr;
wex::vi_macros wex::ex::m_Macros;

wex::ex::ex(stc* stc)
  : m_Command(ex_command(stc))
  , m_CTags(new ctags(this))
  , m_Frame(wxDynamicCast(wxTheApp->GetTopWindow(), managed_frame))
  , m_Commands {
    {":ab", [&](const std::string& command) {
      return HandleContainer<std::string, std::map<std::string, std::string>>(
        "Abbreviations", command, &m_Macros.GetAbbreviations(),
        [=](const std::string& name, const std::string& value) {
          m_Macros.SetAbbreviation(name, value);return true;});}},
    {":ar", [&](const std::string& command) {
      wxString text;
      for (size_t i = 1; i < wxTheApp->argv.GetArguments().size(); i++)
      {
        text << wxTheApp->argv.GetArguments()[i] << "\n";
      }
      if (!text.empty()) ShowDialog("ar", text.ToStdString());
      return true;}},
    {":chd", [&](const std::string& command) {
      if (command.find(" ") == std::string::npos) return true;
      wex::path::Current(wex::firstof(command, " ")); return true;}},
    {":close", [&](const std::string& command) {POST_COMMAND( wxID_CLOSE ) return true;}},
    {":de", [&](const std::string& command) {
      m_Frame->GetDebug()->Execute(wex::firstof(command, " "), m_Command.STC());
      return true;}},
    {":e", [&](const std::string& command) {POST_COMMAND( wxID_OPEN ) return true;}},
    {":f", [&](const std::string& command) {InfoMessage(); return true;}},
    {":grep", [&](const std::string& command) {POST_COMMAND( ID_TOOL_REPORT_FIND ) return true;}},
    {":gt", [&](const std::string& command) {return m_Command.STC()->LinkOpen();}},
    {":help", [&](const std::string& command) {POST_COMMAND( wxID_HELP ) return true;}},
    {":map", [&](const std::string& command) {
      switch (ParseCommandWithArg(command))
      {
        case wex::commandarg::INT:
          // TODO: at this moment you cannot set KEY_CONTROL
          return HandleContainer<int, wex::vi_macros_maptype>(
            "Map", command, nullptr,
            [=](const std::string& name, const std::string& value) {
              m_Macros.SetKeyMap(name, value);return true;}); 
        break;
        case wex::commandarg::NONE: ShowDialog("Maps", 
            "[String map]\n" +
            ReportContainer<std::string, std::map<std::string, std::string>>(m_Macros.GetMap()) +
            "[Key map]\n" +
            ReportContainer<int, wex::vi_macros_maptype>(m_Macros.GetKeysMap()) +
            "[Alt key map]\n" +
            ReportContainer<int, wex::vi_macros_maptype>(m_Macros.GetKeysMap(KEY_ALT)) +
            "[Control key map]\n" +
            ReportContainer<int, wex::vi_macros_maptype>(m_Macros.GetKeysMap(KEY_CONTROL)), 
            true);
          return true;
        break;
        case wex::commandarg::OTHER:
          return HandleContainer<std::string, std::map<std::string, std::string>>(
            "Map", command, nullptr,
            [=](const std::string& name, const std::string& value) {
              m_Macros.SetMap(name, value);return true;});
      }
      return false;}},
    {":new", [&](const std::string& command) {POST_COMMAND( wxID_NEW ) return true;}},
    {":print", [&](const std::string& command) {m_Command.STC()->Print(command.find(" ") == std::string::npos); return true;}},
    {":pwd", [&](const std::string& command) {wex::log_status(wex::path::Current()); return true;}},
    {":q!", [&](const std::string& command) {POST_CLOSE( wxEVT_CLOSE_WINDOW, false ) return true;}},
    {":q", [&](const std::string& command) {POST_CLOSE( wxEVT_CLOSE_WINDOW, true ) return true;}},
    {":reg", [&](const std::string& command) {
      ShowDialog("Registers", m_Evaluator.GetInfo(this), true);
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
             wxConfigBase::Get()->Write(_("Auto complete"), on);}},
           {{"ai", "autoindent"}, [](bool on){
             wxConfigBase::Get()->Write(_("Auto indent"), on ? 2: 0);}},
           {{"aw", "autowrite"}, [&](bool on){
             wxConfigBase::Get()->Write(_("Auto write"), on);
             m_AutoWrite = on;}},
           {{"eb", "errorbells"}, [](bool on){
             wxConfigBase::Get()->Write(_("Error bells"), on);}},
           {{"el", "edgeline"}, [&](bool on){
             m_Command.STC()->SetEdgeMode(
               on ? wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);
             wxConfigBase::Get()->Write(_("Edge line"), 
               on ? wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);}},
           {{"ic", "ignorecase"}, [&](bool on){
             if (!on) m_SearchFlags |= wxSTC_FIND_MATCHCASE;
             else     m_SearchFlags &= ~wxSTC_FIND_MATCHCASE;
             wex::find_replace_data::Get()->SetMatchCase(!on);}},
           {{"mw", "matchwords"}, [&](bool on){
             if (on) m_SearchFlags |= wxSTC_FIND_WHOLEWORD;
             else    m_SearchFlags &= ~wxSTC_FIND_WHOLEWORD;
             wex::find_replace_data::Get()->SetMatchWord(on);}},
           {{"nu", "number"}, [&](bool on){
             m_Command.STC()->ShowLineNumbers(on);
             wxConfigBase::Get()->Write(_("Line numbers"), on);}},
           {{"readonly", "readonly"}, [&](bool on){
             m_Command.STC()->SetReadOnly(on);}},
           {{"showmode", "showmode"}, [&](bool on){
             ((wex::statusbar *)m_Frame->GetStatusBar())->ShowField("PaneMode", on);
             wxConfigBase::Get()->Write(_("Show mode"), on);}},
           {{"sm", "showmatch"}, [&](bool on){
             wxConfigBase::Get()->Write(_("Show match"), on);}},
           {{"sws", "showwhitespace"}, [&](bool on){
             m_Command.STC()->SetViewEOL(on);
             m_Command.STC()->SetViewWhiteSpace(on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);
             wxConfigBase::Get()->Write(_("Whitespace"), on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);}},
           {{"ut", "usetabs"}, [&](bool on){
             m_Command.STC()->SetUseTabs(on);
             wxConfigBase::Get()->Write(_("Use tabs"), on);}},
           {{"wm", "wrapmargin"}, [&](bool on){
             m_Command.STC()->SetWrapMode(on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);
             wxConfigBase::Get()->Write(_("Wrap line"), on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);}},
           {{"ws", "wrapscan", "1"}, [&](bool on){
             wxConfigBase::Get()->Write(_("Wrap scan"), on);}}
          },

          // options
          {
           {{"dir", "dir"}, {CMD_LINE_STRING, [&](const std::any& val) {
             wex::path::Current(std::any_cast<std::string>(val));}}},
           {{"ec", "edgecolumn", "80"}, {CMD_LINE_INT, [&](const std::any& val) {
             m_Command.STC()->SetEdgeColumn(std::any_cast<int>(val));
             wxConfigBase::Get()->Write(_("Edge column"), std::any_cast<int>(val));}}},
           {{"report", "report", "5"}, {CMD_LINE_INT, [&](const std::any& val) {
             wxConfigBase::Get()->Write("Reported lines", std::any_cast<int>(val));}}},
           {{"sw", "shiftwidth", "8"}, {CMD_LINE_INT, [&](const std::any& val) {
             m_Command.STC()->SetIndent(std::any_cast<int>(val));
             wxConfigBase::Get()->Write(_("Indent"), std::any_cast<int>(val));}}},
           {{"sy", "syntax (lexer or 'off')"}, {CMD_LINE_STRING, [&](const std::any& val) {
             if (std::any_cast<std::string>(val) != "off") 
               m_Command.STC()->GetLexer().Set(std::any_cast<std::string>(val), true); // allow folding
             else              
               m_Command.STC()->GetLexer().Reset();}}},
           {{"ts", "tabstop","8"}, {CMD_LINE_INT, [&](const std::any& val) {
             m_Command.STC()->SetTabWidth(std::any_cast<int>(val));
             wxConfigBase::Get()->Write(_("Tab width"), std::any_cast<int>(val));}}}
          }
        );

      if (command.find(" ") == std::string::npos)
      {
        cmdline.ShowOptions(wex::window_data().Size({200, 450}));
      }
      else
      {
        cmdline.Parse(command.substr(0, 4) + text, save);
      }
      return true;}},
    {":so", [&](const std::string& command) {
      if (command.find(" ") == std::string::npos) return false;
      wex::path path(wex::firstof(command, " "));
      if (path.IsRelative())
      {
        path.MakeAbsolute();
      }
      std::ifstream ifs(path.Path());
      if (!ifs.is_open()) return false;
      int i = 0;
      for (std::string line; std::getline(ifs, line); )
      {
        if (!line.empty())
        {
          if (line == command)
          {
            VLOG(9) << "recursive (line: " << i + 1 << ")";
            return false;
          }
          else if (!Command(line))
          {
            VLOG(9) << "command error (line: " << i + 1 << ")";
            return false;
          }
        }
        i++;
      }
      return true;}},
    {":syntax", [&](const std::string& command) {
      if (wxString(command).EndsWith("on"))
      {
        wex::lexers::Get()->RestoreTheme();
        m_Command.STC()->GetLexer().Set(m_Command.STC()->GetFileName().GetLexer().GetDisplayLexer(), true); // allow folding
      }
      else if (wxString(command).EndsWith("off"))
      {
        m_Command.STC()->GetLexer().Reset();
        wex::lexers::Get()->ResetTheme();
      }
      else
      {
        return false;
      }
      m_Frame->StatusText(wex::lexers::Get()->GetTheme(), "PaneTheme");
      return true;}},
    {":ta", [&](const std::string& command) {
      m_CTags->Find(wex::firstof(command, " "));
      return true;}},
    {":una", [&](const std::string& command) {
      if (wex::tokenizer tkz(command); tkz.CountTokens() >= 1)
      {
        tkz.GetNextToken(); // skip :una
        m_Macros.SetAbbreviation(tkz.GetNextToken(), "");
      }
      return true;}},
    {":unm", [&](const std::string& command) {
      if (wex::tokenizer tkz(command); tkz.CountTokens() >= 1)
      {
        tkz.GetNextToken(); // skip :unm
        switch (ParseCommandWithArg(command))
        {
          case wex::commandarg::INT: m_Macros.SetKeyMap(tkz.GetNextToken(), ""); break; 
          case wex::commandarg::NONE: break;
          case wex::commandarg::OTHER: m_Macros.SetMap(tkz.GetNextToken(), ""); break;
        }
      }
      return true;}},
    {":ve", [&](const std::string& command) {ShowDialog("Version", 
      wex::get_version_info().Get()); return true;}},
    {":x", [&](const std::string& command) {
      if (command != ":x") return false;
      POST_COMMAND( wxID_SAVE )
      POST_CLOSE( wxEVT_CLOSE_WINDOW, true )
      return true;}}}
{
  wxASSERT(m_Frame != nullptr);
  ResetSearchFlags();
  m_AutoWrite = wxConfigBase::Get()->ReadLong(_("Auto write"), 0);
}

wex::ex::~ex()
{
  delete m_CTags;
}
  
void wex::ex::AddText(const std::string& text)
{
  if (m_Register)
  {
    m_Macros.SetRegister(m_Register, text);
  }
  else
  {
    m_Command.STC()->AddTextRaw((const char *)text.c_str(), text.length());
  }

  InfoMessage(text, wex::info_message::ADD);
}

bool wex::ex::AutoWrite()
{
  if (!m_AutoWrite || !m_Command.STC()->IsModified())
  {
    return true;
  }

  wxCommandEvent event(
    wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE);

  wxPostEvent(wxTheApp->GetTopWindow(), event);

  return true;
}

std::tuple<double, int> wex::ex::Calculator(const std::string& text)
{
  const auto& [val, width, err] = m_Evaluator.Eval(this, text);

  if (!err.empty())
  {
    ShowDialog("Error", text + "\n" + err);
  }

  return {val, width};
}

bool wex::ex::Command(const std::string& cmd)
{
  auto command(cmd);

  if (!m_IsActive || command.empty() || command.front() != ':') return false;

  VLOG(9) << "ex command: " << cmd;

  const auto& it = m_Macros.GetMap().find(command);
  command = (it != m_Macros.GetMap().end() ? it->second: command);

  if (m_Frame->ExecExCommand(m_Command.Command(cmd)))
  {
    m_Command.clear();
    return AutoWrite();
  }
  else if (command == ":" || command == ":'<,'>" || command == ":!")
  {
    return m_Frame->GetExCommand(this, command);
  }
  else if (
    !CommandHandle(command) &&
    !CommandAddress(command.substr(1)))
  {
    m_Command.clear();
    return false;
  }

  m_Macros.Record(command);
  m_Command.clear();

  return AutoWrite();
}

bool wex::ex::CommandAddress(const std::string& command)
{
  auto rest(command);
  std::string range_str, cmd;
  bool addr1 = false; // single address

  if (rest.compare(0, 5, "'<,'>") == 0)
  {
    if (GetSelectedText().empty())
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
          rest = skip_white_space(v[2], SKIP_LEFT);
          break;
        case 4:
          range_str = v[0] + v[1];
          cmd = v[2];
          rest = v[3];
          break;
        default: wxFAIL; break;
      }

      if (!marker_and_register_expansion(this, range_str))
      {
        return false;
      }
    }
    else 
    {
      const auto line(address(this, rest).GetLine());
      if (line > 0) stc_data(m_Command.STC()).Control(control_data().Line(line)).Inject();
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
    case 'a': return addr.Append(rest);
    case 'i': return addr.Insert(rest);
    case 'k': return !rest.empty() ? addr.MarkerAdd(rest[0]): false;
    case 'p': 
      if (cmd == "pu")
      { 
        return !rest.empty() ? addr.Put(rest[0]): addr.Put();
      }
      else
      {
        return false;
      }
      break;
    case 'r': return addr.Read(rest);
    case 'z': return addr.AdjustWindow(rest);
    case '=': return addr.WriteLineNumber();
    default:
      wxLogStatus("Unknown address command: %s", cmd);
      return false;
    }
  }
  else
  {
    switch (addressrange range(this, range_str); (int)cmd[0])
    {
    case 0: return false;
    case 'c': return range.Change(rest);
    case 'd': return range.Delete();
    case 'v':
    case 'g': return range.Global(rest, cmd[0] == 'v');
    case 'j': return range.Join();
    case 'm': return range.Move(address(this, rest));
    case 'p': 
      if (m_Command.STC()->GetName() != "Print")
      {
        return range.Print(rest);
      }
      else
      {
        return false;
      }
    case 's':
    case '&':
    case '~': return range.Substitute(rest, cmd[0]);
    case 'S': return range.Sort(rest);
    case 't': return range.Copy(address(this, rest));
    case 'w': 
      if (!rest.empty())
      {
        return range.Write(rest);
      }
      else
      {
        POST_COMMAND( wxID_SAVE )
        return true;
      }
      break;
    case 'y': return range.Yank(rest.empty() ? '0': (char)rest[0]);
    case '>': return range.ShiftRight();
    case '<': return range.ShiftLeft();
    case '!': return range.Escape(rest);
    default:
      wxLogStatus("Unknown range command: %s", cmd);
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

void wex::ex::Copy(const wex::ex* ex)
{
  m_MarkerIdentifiers = ex->m_MarkerIdentifiers;
  m_Copy = true; // no char numbers for a copy
}

void wex::ex::Cut(bool show_message)
{
  const auto sel(GetSelectedText());
  
  Yank('0', false);

  m_Command.STC()->ReplaceSelection(wxEmptyString);
  
  SetRegistersDelete(sel);
  
  InfoMessage(sel, wex::info_message::DEL);
}

const std::string wex::ex::GetRegisterInsert() const
{
  return m_Macros.GetRegister('.');
}

const std::string wex::ex::GetRegisterText() const
{
  return m_Register ? 
    m_Macros.GetRegister(m_Register):
    m_Macros.GetRegister('0');
}
  
const std::string wex::ex::GetSelectedText() const
{
  // This also supports rectangular text.
  if (m_Command.STC()->GetSelectedText().empty())
  {
    return std::string();
  }

  const wxCharBuffer b(m_Command.STC()->GetSelectedTextRaw());
  return std::string(b.data(), b.length() - 1);
}

template <typename S, typename T> 
bool wex::ex::HandleContainer(
  const std::string& kind,
  const std::string& command,
  const T * container,
  std::function<bool(const std::string&, const std::string&)> cb)
{
  if (tokenizer tkz(command); tkz.CountTokens() >= 2)
  {
    tkz.GetNextToken(); // skip
    const auto name(tkz.GetNextToken());
    cb(name, tkz.GetString());
  }
  else if (container != nullptr)
  {
    ShowDialog(kind, ReportContainer<S, T>(*container), true);
  }

  return true;
}

void wex::ex::InfoMessage() const
{
  m_Frame->ShowExMessage(wxString::Format("%s line %d of %d --%d%%-- level %d", 
    m_Command.STC()->GetFileName().GetFullName().c_str(), 
    m_Command.STC()->GetCurrentLine() + 1,
    m_Command.STC()->GetLineCount(),
    100 * (m_Command.STC()->GetCurrentLine() + 1)/ m_Command.STC()->GetLineCount(),
    (m_Command.STC()->GetFoldLevel(m_Command.STC()->GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK)
     - wxSTC_FOLDLEVELBASE).ToStdString());
}

void wex::ex::InfoMessage(const std::string& text, wex::info_message type) const
{
  if (const auto lines = get_number_of_lines(text);
    lines >= wxConfig::Get()->Read("Reported lines", 5))
  {
    wxString msg;

    switch (type)
    {
      case wex::info_message::ADD: msg = _("%d lines added"); break;
      case wex::info_message::COPY: msg = _("%d lines yanked"); break;
      case wex::info_message::DEL: msg = _("%d fewer lines"); break;
    }

    m_Frame->ShowExMessage(wxString::Format(msg, lines - 1).ToStdString());
  }
}

bool wex::ex::MarkerAdd(char marker, int line)
{
  if (m_Copy) return false;

  const wex::marker lm(wex::lexers::Get()->GetMarker(m_MarkerSymbol));

  if (!lm.IsOk())
  {
    wex::log("could not find marker symbol") << m_MarkerSymbol.GetNo() << " in lexers";
    return false;
  }
  
  MarkerDelete(marker);

  int id;
  const int lin = (line == -1 ? m_Command.STC()->GetCurrentLine(): line);

  if (lm.GetSymbol() == wxSTC_MARK_CHARACTER)
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

      m_Command.STC()->MarkerDefine(marker_number, 
        wxSTC_MARK_CHARACTER + marker, 
        wxString(lm.GetForegroundColour()), 
        wxString(lm.GetBackgroundColour()));

      id = m_Command.STC()->MarkerAdd(lin, marker_number);
      m_MarkerNumbers[marker] = marker_number;
    }
    else
    {
      id = m_Command.STC()->MarkerAdd(lin, it->second);
    }
  }
  else
  {
    id = m_Command.STC()->MarkerAdd(lin, m_MarkerSymbol.GetNo());
  }
    
  if (id == -1)
  {
    log("could not add marker") << marker  << "to line:" << lin;
    return false;  
  }
    
  m_MarkerIdentifiers[marker] = id;
  
  return true;
}  

bool wex::ex::MarkerDelete(char marker)
{
  if (const auto& it = m_MarkerIdentifiers.find(marker); it != m_MarkerIdentifiers.end())
  {
    m_Command.STC()->MarkerDeleteHandle(it->second);
    m_MarkerIdentifiers.erase(it);
    return true;
  }
  
  return false;
}

bool wex::ex::MarkerGoto(char marker)
{
  if (const auto line = MarkerLine(marker); line != -1)
  {
    stc_data(m_Command.STC()).Control(control_data().Line(line + 1)).Inject();
    return true;
  }
  
  return false;
}

int wex::ex::MarkerLine(char marker) const
{
  if (marker == '<')
  {
    if (!GetSelectedText().empty())
    {
      return m_Command.STC()->LineFromPosition(m_Command.STC()->GetSelectionStart());
    }
  }
  else if (marker == '>')
  {
    if (!GetSelectedText().empty())
    {
      return m_Command.STC()->LineFromPosition(m_Command.STC()->GetSelectionEnd());
    }
  }
  else
  {
    if (const auto& it = m_MarkerIdentifiers.find(marker); it != m_MarkerIdentifiers.end())
    {
      if (const auto line = m_Command.STC()->MarkerLineFromHandle(it->second); line == -1)
      {
        wxLogStatus("Handle for marker: %c invalid", marker);
      }
      else
      {
        return line;
      }
    }
    else
    {
      wxLogStatus(_("Undefined marker: %c"), marker);
    }
  }

  if (wxConfigBase::Get()->ReadLong(_("Error bells"), 1))
  {
    wxBell();
  }

  return -1;
}

void wex::ex::Print(const std::string& text)
{
  ShowDialog("Print", text);
}
  
template <typename S, typename T>
std::string wex::ex::ReportContainer(const T & t) const
{
  const wex::lexer_props l;
  std::string output;

  for (const auto& it : t)
  {
    output += l.MakeKey(type_to_value<S>(it.first).getString(), it.second);
  }

  return output;
}

void wex::ex::ResetSearchFlags()
{
  m_SearchFlags = ((find_replace_data::Get()->MatchCase() ? 
    wxSTC_FIND_MATCHCASE: 0) | 
    wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX);
}

void wex::ex::SetRegistersDelete(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  for (int i = 9; i >= 2; i--)
  {
    if (const auto value(m_Macros.GetRegister(wxUniChar(48 + i - 1)));
      !value.empty())
    {
      m_Macros.SetRegister(wxUniChar(48 + i), value);
    }
  }
  
  m_Macros.SetRegister('1', value);
}
  
void wex::ex::SetRegisterInsert(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  m_Macros.SetRegister('.', value);
}

void wex::ex::SetRegisterYank(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  m_Macros.SetRegister('0', value);
}

void wex::ex::ShowDialog(
  const std::string& title, const std::string& text, bool prop_lexer)
{
  if (m_Dialog == nullptr)
  {
    m_Dialog = new stc_entry_dialog(
      text,
      std::string(),
      window_data().Button(wxOK).Title(title));
  }
  else
  {
    if (title == "Print")
    { 
      if (title != m_Dialog->GetTitle())
      {
        m_Dialog->GetSTC()->SetText(text);
      }
      else
      {
        m_Dialog->GetSTC()->AppendText(text);
        m_Dialog->GetSTC()->DocumentEnd();
      }
    }
    else
    {
      m_Dialog->GetSTC()->SetText(text);
    }
    
    m_Dialog->SetTitle(title);
  }
  
  m_Dialog->GetSTC()->GetLexer().Set(
    prop_lexer ? lexer_props(): m_Command.STC()->GetLexer());
  m_Dialog->Show();
}

bool wex::ex::Yank(const char name, bool show_message) const
{
  const auto range(GetSelectedText());
  
  if (range.empty())
  {
    return false;
  }
  else if (GetRegister())
  {
    m_Macros.SetRegister(GetRegister(), range);
  }
  else if (name != '0')
  {
    m_Macros.SetRegister(name, range);
  }
  else
  {
    SetRegisterYank(range);
  }

  InfoMessage(range, wex::info_message::COPY);
  
  return true;
}
