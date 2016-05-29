////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wxExEx
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/ex.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <shunting-yard/eval.hpp>
#include <wx/config.h>
#include <wx/numformatter.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/extension/ex.h>
#include <wx/extension/address.h>
#include <wx/extension/addressrange.h>
#include <wx/extension/cmdline.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

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

class ex_evaluator : public evaluator_extra 
{
public:
  ex_evaluator() 
  {
    // prevent a comma to be used as argument separator
    // for functions
    opers.insert({",", oper_t{false, 1, false}});
    opers.insert({">>", oper_t{false, 10, false}});
    opers.insert({"<<", oper_t{false, 10, false}});
    opers.insert({"&", oper_t{false, 10, false}});
    opers.insert({"|", oper_t{false, 10, false}});
    opers.insert({"xor", oper_t{false, 10, false}});
    opers.insert({"bitor", oper_t{false, 10, false}});
    opers.insert({"bitand", oper_t{false, 10, false}});
    
    funcs.insert({",", func_args(2, [](args_t v) {
      return v[0] + v[1] / 10;})});
    funcs.insert({">>", func_args(2, [](args_t v) {
      return (int)v[0] >> (int)v[1];})});
    funcs.insert({"<<", func_args(2, [](args_t v) {
      return (int)v[0] << (int)v[1]; })});
    funcs.insert({"&", func_args(2, [](args_t v) {
      return (int)v[0] & (int)v[1]; })});
    funcs.insert({"|", func_args(2, [](args_t v) {
      return (int)v[0] | (int)v[1]; })});
    funcs.insert({"compl", func_args(1, [](args_t v) {
      return ~(int)v[0];})});
    funcs.insert({"xor", func_args(2, [](args_t v) {
      return (int)v[0] ^ (int)v[1]; })});
    funcs.insert({"bitor", func_args(2, [](args_t v) {
      return (int)v[0] | (int)v[1]; })});
    funcs.insert({"bitand", func_args(2, [](args_t v) {
      return (int)v[0] & (int)v[1]; })});
  }
};

ex_evaluator wxExEx::m_Evaluator;
wxExSTCEntryDialog* wxExEx::m_Dialog = nullptr;
wxExViMacros wxExEx::m_Macros;
std::string wxExEx::m_LastCommand;

wxExEx::wxExEx(wxExSTC* stc)
  : m_STC(stc)
  , m_Frame(wxDynamicCast(wxTheApp->GetTopWindow(), wxExManagedFrame))
  , m_IsActive(true)
  , m_SearchFlags(wxExFindReplaceData::Get()->MatchCase() ? 
      wxSTC_FIND_MATCHCASE | wxSTC_FIND_REGEXP: wxSTC_FIND_REGEXP)
  , m_MarkerSymbol(0, -1)
  , m_Register(0)
  , m_Commands {
    {":ab", [&](const std::string& command) {
      wxStringTokenizer tkz(command, " ");
      if (tkz.CountTokens() >= 2)
      {
        tkz.GetNextToken(); // skip
        const wxString ab(tkz.GetNextToken());
        m_Macros.SetAbbreviation(ab, tkz.GetString().ToStdString());
      }
      else
      {
        wxString output;
        for (const auto& it : m_Macros.GetAbbreviations())
        {
          output << it.first << "=" << it.second << "\n";
        }
        ShowDialog("Abbreviations", output, true);
      } return true;}},
#if wxCHECK_VERSION(3,1,0)
    {":ar", [&](const std::string& command) {
      wxString text;
      for (size_t i = 1; i < wxTheApp->argv.GetArguments().size(); i++)
      {
        text << wxTheApp->argv.GetArguments()[i] << "\n";
      }
      if (!text.empty()) ShowDialog("ar", text);
      return true;}},
#endif
    {":chd", [&](const std::string& command) {
      if (command.find(" ") == std::string::npos) return true;
      wxSetWorkingDirectory(wxString(command).AfterFirst(' ').Trim(false)); return true;}},
    {":close", [&](const std::string& command) {POST_COMMAND( wxID_CLOSE ) return true;}},
    {":e", [&](const std::string& command) {POST_COMMAND( wxID_OPEN ) return true;}},
    {":grep", [&](const std::string& command) {POST_COMMAND( ID_TOOL_REPORT_FIND ) return true;}},
    {":help", [&](const std::string& command) {POST_COMMAND( wxID_HELP ) return true;}},
    {":new", [&](const std::string& command) {POST_COMMAND( wxID_NEW ) return true;}},
    {":print", [&](const std::string& command) {m_STC->Print(command.find(" ") == std::string::npos); return true;}},
    {":pwd", [&](const std::string& command) {wxLogStatus(wxGetCwd()); return true;}},
    {":q", [&](const std::string& command) {POST_CLOSE( wxEVT_CLOSE_WINDOW, true ) return true;}},
    {":q!", [&](const std::string& command) {POST_CLOSE( wxEVT_CLOSE_WINDOW, false ) return true;}},
    {":reg", [&](const std::string& command) {
      wxString output;
      for (const auto& it : m_Macros.GetRegisters())
      {
        output << it << "\n";
      }
      output << "%: " << m_STC->GetFileName().GetFullName() << "\n";
      std::string err;
      for (const auto &var : m_Evaluator.variables) 
      {
        output << var << "=" << m_Evaluator.eval(var, &err) << "\n";
      }
      ShowDialog("Registers", output, true);
      return true;}},
    {":sed", [&](const std::string& command) {POST_COMMAND( ID_TOOL_REPORT_REPLACE ) return true;}},
    {":set", [&](const std::string& command) {
      if (command.find(" ") == std::string::npos)
      {
        POST_COMMAND( wxID_PREFERENCES )
        return true;
      }
      else
      {
        wxString text(wxString(command.substr(4)).Trim(false));
        if (!text.Contains("/") && (text.Contains("=") || !text.Contains("-")))
        {
          // Convert modeline to commandline arg (add - to each group, remove all =).
          // ts=120 ac ic sy=cpp -> -ts 120 -ac -ic -sy cpp
          std::regex re("[0-9a-z=]+");
          text = std::regex_replace(text.ToStdString(), re, "-&", std::regex_constants::format_sed);
          text.Replace("=", " ");
          text.Replace("!", "-"); // change negatable char
        }
        return wxExCmdLineParser(text, 
          wxExCmdLineParser::CmdSwitches { 
            {{"ac", "Auto Complete"}, {wxCMD_LINE_SWITCH_NEGATABLE, [](bool on){wxConfigBase::Get()->Write(_("Auto complete"), on);}}},
            {{"ai", "Auto Indent"}, {wxCMD_LINE_SWITCH_NEGATABLE, [](bool on){wxConfigBase::Get()->Write(_("Auto indent"), on ? 2: 0);}}},
            {{"el", "Edge Line"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              m_STC->SetEdgeMode(on ? wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);     
              wxConfigBase::Get()->Write(_("Edge line"), on ? wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);}}},
            {{"ic", "Ignore Case"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              if (!on) m_SearchFlags |= wxSTC_FIND_MATCHCASE;
              else     m_SearchFlags &= ~wxSTC_FIND_MATCHCASE;
              wxExFindReplaceData::Get()->SetMatchCase(!on);}}},
            {{"mw", "Match Words"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              if (on) m_SearchFlags |= wxSTC_FIND_WHOLEWORD;
              else    m_SearchFlags &= ~wxSTC_FIND_WHOLEWORD;
              wxExFindReplaceData::Get()->SetMatchWord(on);}}},
            {{"nu", "show LineNumbers"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              m_STC->ShowLineNumbers(on);
              wxConfigBase::Get()->Write(_("Line numbers"), on);}}},
            {{"re", "Regular Expression"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              if (on) m_SearchFlags |= wxSTC_FIND_REGEXP;
              else    m_SearchFlags &= ~wxSTC_FIND_REGEXP;
              wxExFindReplaceData::Get()->SetUseRegEx(on);}}},
            {{"sm", "Show Mode"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              ((wxExStatusBar *)m_Frame->GetStatusBar())->ShowField("PaneMode", on);
              wxConfigBase::Get()->Write(_("Show mode"), on);}}},
            {{"ut", "Use Tabs"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              m_STC->SetUseTabs(on);
              wxConfigBase::Get()->Write(_("Use tabs"), on);}}},
            {{"wl", "Wrap Line"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              m_STC->SetWrapMode(on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);
              wxConfigBase::Get()->Write(_("Wrap line"), on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);}}},
            {{"ws", "show WhiteSpace"}, {wxCMD_LINE_SWITCH_NEGATABLE, [&](bool on){
              m_STC->SetViewEOL(on);
              m_STC->SetViewWhiteSpace(on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);
              wxConfigBase::Get()->Write(_("Whitespace"), on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);}}}},
          wxExCmdLineParser::CmdOptions {
            {{"ec", "Edge Column"}, {wxCMD_LINE_VAL_NUMBER, [&](wxAny any) {
              long val;
              any.GetAs(&val);
              m_STC->SetEdgeColumn(val);
              wxConfigBase::Get()->Write(_("Edge column"), val);}}},
            {{"sw", "Shift Width"}, {wxCMD_LINE_VAL_NUMBER, [&](wxAny any) {
              long val;
              any.GetAs(&val);
              m_STC->SetIndent(val);
              wxConfigBase::Get()->Write(_("Indent"), val);}}},
            {{"sy", "SYntax (off)"}, {wxCMD_LINE_VAL_STRING, [&](wxAny any) {
              wxString val;
              any.GetAs(&val);
              if (val != "off") m_STC->GetLexer().Set(val, true); // allow folding
              else              m_STC->GetLexer().Reset();}}},
            {{"ts", "Tab Stop"}, {wxCMD_LINE_VAL_NUMBER, [&](wxAny any) {
              long val;
              any.GetAs(&val);
              m_STC->SetTabWidth(val);
              wxConfigBase::Get()->Write(_("Tab width"), val);}}}}).Parse() <= 0;
      }}},
    {":so", [&](const std::string& command) {
      if (command.find(" ") == std::string::npos) return false;
      wxFileName filename(wxString(command).AfterFirst(' ').Trim(false));
      if (filename.IsRelative())
      {
        filename.MakeAbsolute();
      }
      wxTextFile file(filename.GetFullPath());
      if (!file.Open()) return false;
      for (int i = 0; i < (int)file.GetLineCount(); i++)
      {
        const std::string line(file.GetLine(i).ToStdString());
        if (!line.empty())
        {
          if (line == command)
          {
            wxLogMessage("skip recursive %s on line %d", line.c_str(), i);
            return false;
          }
          else if (!Command(line))
          {
            wxLogMessage("%s on line %d failed", line.c_str(), i + 1);
            return false;
          }
        }
      }
      return true;}},
    {":syntax", [&](const std::string& command) {
      if (wxString(command).EndsWith("on"))
      {
        wxExLexers::Get()->RestoreTheme();
        m_STC->GetLexer().Set(m_STC->GetFileName().GetLexer().GetDisplayLexer(), true); // allow folding
      }
      else if (wxString(command).EndsWith("off"))
      {
        m_STC->GetLexer().Reset();
        wxExLexers::Get()->SetThemeNone();
      }
      else
      {
        return false;
      }
      m_Frame->StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
      return true;}},
    {":una", [&](const std::string& command) {
      wxStringTokenizer tkz(command, " ");
      if (tkz.CountTokens() >= 2)
      {
        tkz.GetNextToken(); // skip
        const wxString ab(tkz.GetNextToken());
        m_Macros.SetAbbreviation(ab, "");
      }
      return true;}},
    {":ve", [&](const std::string& command) {ShowDialog("Version", wxExGetVersionInfo().GetVersionOnlyString()); return true;}},
    {":x", [&](const std::string& command) {
      if (command != ":x") return false;
      POST_COMMAND( wxID_SAVE )
      POST_CLOSE( wxEVT_CLOSE_WINDOW, true )
      return true;}}}
{
  wxASSERT(m_Frame != nullptr);
}

void wxExEx::AddText(const std::string& text)
{
  if (m_Register)
  {
    m_Macros.SetRegister(m_Register, text);
  }
  else
  {
    m_STC->AddText(text);
  }
}

double wxExEx::Calculator(const std::string& text, int& width)
{
  wxString expr(text);
  expr.Trim();

  if (expr.empty() || expr.Contains("%s"))
  {
    return 0;
  }
  
  const char ds(wxNumberFormatter::GetDecimalSeparator());
  
  // Determine the width.
  const std::string rt((ds == '.' ? "\\.": std::string(1, ds)) + std::string("[0-9]+"));
  std::regex re(rt);
  const auto words_begin = std::sregex_iterator(text.begin(), text.end(), re);
  const auto words_end = std::sregex_iterator();  

  if (words_begin != words_end)
  {
    std::smatch match = *words_begin; 

    if (!match.empty())
    {
      width = match.length() - 1;
    }
  }
  else
  {
    width = 0;
    
    // Replace . with current line.
    expr.Replace(".", std::to_string(m_STC->GetCurrentLine() + 1));
  }
  
  // Replace $ with line count.
  expr.Replace("$", std::to_string(m_STC->GetLineCount()));
  
  // Expand all markers and registers.
  if (!wxExMarkerAndRegisterExpansion(this, expr))
  {
    return 0;
  }

  // https://github.com/r-lyeh/eval
  std::string err;
  auto val = m_Evaluator.eval(expr.ToStdString(), &err);
  if (!err.empty())
  {
    ShowDialog("Error", expr + "\n" + wxString(err));
    val = 0;
  }

  return val;
}

bool wxExEx::Command(const std::string& command, bool is_handled)
{
  if (!m_IsActive || command.empty())
  {
    return false;
  }

  wxExSTC* stc = nullptr;

  if (m_Frame->ExecExCommand(command, stc))
  {
    if (stc != nullptr)
    {
      m_STC = stc;
    }
    return true;
  }
  if (command.front() != ':')
  {
    return false;
  }
  else if (command == ":" || command == ":'<,'>")
  {
    m_Frame->GetExCommand(this, command);
    return true;
  }
  else if (CommandHandle(command) ||
           CommandAddress(command.substr(1)))
  {
    SetLastCommand(command);
    m_Macros.Record(command);
    return true;
  }
  else
  {
    return false;
  }
}

bool wxExEx::CommandAddress(const std::string& command)
{
  wxString rest(command);
  wxString range_str;  
  wxString cmd;
  bool addr1 = false; // single address

  if (rest.compare(0, 5, "'<,'>") == 0)
  {
    if (GetSelectedText().empty())
    {
      return false;
    }

    range_str = "'<,'>";
    cmd = rest.Mid(5);
    rest = rest.Mid(6);
  }
  else
  { 
    const std::string addr("[0-9\\.\\$\\+\\-]+"); // addr (normal)
    const std::string addrs("[\\?/].*?[\\?/]"); // addr search, non-greedy!
    const std::string addrm("'[a-z]"); // addr using marker
    const std::string cmd_group1("([aikrz=]|pu)(.*)"); // 1 addr command
    const std::string cmd_group2("([cdgjmpsStvywy<>\\!&~])(.*)"); // 2 addr command
    std::vector <wxString> v;
    
    if (
      // a % address range
      wxExMatch("^%" + cmd_group2, rest.ToStdString(), v) == 2 ||
      // addr2 search
      wxExMatch("^(" + addrs + ")(," + addrs + ")" + cmd_group2, rest.ToStdString(), v) == 4 ||
      // addr1 search
      wxExMatch("^(" + addrs + ")" + cmd_group1, rest.ToStdString(), v) == 3 ||
      // addr2 markers
      wxExMatch("^(" + addrm + ")(," + addrm + ")" + cmd_group2, rest.ToStdString(), v) == 4 ||
      wxExMatch("^(" + addr  + ")(," + addrm + ")" + cmd_group2, rest.ToStdString(), v) == 4 ||
      wxExMatch("^(" + addrm + ")(," +  addr + ")" + cmd_group2, rest.ToStdString(), v) == 4 ||
      // addr1 marker
      wxExMatch("^(" + addrm + ")" + cmd_group1, rest.ToStdString(), v) == 3 ||
      // addr1
      wxExMatch("^(" + addr + ")?" + cmd_group1, rest.ToStdString(), v) == 3 ||
      // addr2
      wxExMatch("^(" + addr + ")?(," + addr + ")?" + cmd_group2, rest.ToStdString(), v) == 4)
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
          rest = v[2].Trim(false);
          break;
        case 4:
          range_str = v[0] + v[1];
          cmd = v[2];
          rest = v[3];
          break;
        default: wxFAIL; break;
      }

      if (!wxExMarkerAndRegisterExpansion(this, range_str))
      {
        return false;
      }
    }
    else 
    {
      const auto line(wxExAddress(this, rest).GetLine());
      if (line > 0) m_STC->GotoLineAndSelect(line);
      return line > 0;
    }
    
    if (range_str.empty() && cmd != '!') 
    {
      range_str = (cmd == "g" || cmd == 'v' || cmd == 'w' ? "%": ".");
    }
  }
  
  if (addr1)
  {
    const wxExAddress addr(this, range_str);
    
    switch ((int)cmd.GetChar(0))
    {
    case 0: return false; break;
    case 'a': return addr.Append(rest); break;
    case 'i': return addr.Insert(rest); break;
    case 'k': return !rest.empty() ? addr.MarkerAdd(rest.GetChar(0)): false; break;
    case 'p': 
      if (cmd == "pu")
      { 
        return !rest.empty() ? addr.Put(rest.GetChar(0)): addr.Put();
      }
      else
      {
        return false;
      }
      break;
    case 'r': return addr.Read(rest); break;
    case 'z': return addr.AdjustWindow(rest); break;
    case '=': return addr.Show(); break;
    default:
      wxLogStatus("Unknown address command: %s", cmd);
      return false;
    }
  }
  else
  {
    wxExAddressRange range(this, range_str);
    
    switch ((int)cmd.GetChar(0))
    {
    case 0: return false; break;
    case 'c': return range.Change(rest); break;
    case 'd': return range.Delete(); break;
    case 'v':
    case 'g': return range.Global(rest, cmd.GetChar(0) == 'v'); break;
    case 'j': return range.Join(); break;
    case 'm': return range.Move(wxExAddress(this, rest)); break;
    case 'p': 
      if (m_STC->GetName() != "Print")
      {
        return range.Print(rest);
      }
      else
      {
        return false;
      }
    case 's':
    case '&':
    case '~': return range.Substitute(rest, cmd.GetChar(0)); break;
    case 'S': return range.Sort(rest); break;
    case 't': return range.Copy(wxExAddress(this, rest)); break;
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
    case 'y': return range.Yank(rest.empty() ? '0': (char)rest.GetChar(0)); break;
    case '>': return range.Indent(true); break;
    case '<': return range.Indent(false); break;
    case '!': return range.Escape(rest); break;
    default:
      wxLogStatus("Unknown range command: %s", cmd);
      return false;
    }
  }
}

bool wxExEx::CommandHandle(const std::string& command) const
{
  const auto it = std::find_if(m_Commands.begin(), m_Commands.end(), 
    [command](auto const& e) {return e.first == command.substr(0, e.first.size());});
  
  return it != m_Commands.end() && it->second(command);
}

void wxExEx::Cut(bool show_message)
{
  const std::string sel(GetSelectedText());
  const int lines = wxExGetNumberOfLines(sel);
  
  Yank(false);

  m_STC->ReplaceSelection(wxEmptyString);
  
  SetRegistersDelete(sel);
  
  if (lines >= 3 && show_message)
  {
    m_Frame->ShowExMessage(wxString::Format(_("%d fewer lines"), lines - 1));
  }
}

const std::string wxExEx::GetRegisterInsert() const
{
  return m_Macros.GetRegister('.');
}

const std::string wxExEx::GetRegisterText() const
{
  return m_Register ? 
    m_Macros.GetRegister(m_Register):
    m_Macros.GetRegister('0');
}
  
const std::string wxExEx::GetSelectedText() const
{
  // This also supports rectangular text.
  if (m_STC->GetSelectedText().empty())
  {
    std::string none;
    return none;
  }

  const wxCharBuffer b(m_STC->GetSelectedTextRaw());
  return std::string(b.data(), b.length() - 1);
}

bool wxExEx::MacroPlayback(const wxString& macro, int repeat)
{
  if (!m_IsActive)
  {
    return false;
  }
  
  wxString choice(macro);
  
  if (choice.empty())
  {
    const auto& v(m_Macros.Get());
    
    if (v.empty())
    {
      return false;
    }
  
    wxArrayString macros;
    macros.resize(v.size());
    copy(v.begin(), v.end(), macros.begin());
    
    wxSingleChoiceDialog dialog(m_STC,
      _("Input") + ":", 
      _("Select Macro"),
      macros);
      
    const int index = macros.Index(m_Macros.GetMacro());
  
    if (index != wxNOT_FOUND)
    {
      dialog.SetSelection(index);
    }

    if (dialog.ShowModal() != wxID_OK)
    {
      return false;
    }
    
    choice = dialog.GetStringSelection();
  }
  
  // The STC pointer might change because of playback.
  // Normally that is ok, but if playback fails, and
  // the m_STC was changed (because of e.g. :n), 
  // you would end up with an incorrect pointer.
  // Therefore set it back after the playback.
  wxExSTC* stc = m_STC;

  bool ok = true;
  
  if (m_Macros.IsRecordedMacro(choice))
  {
    ok = m_Macros.Playback(this, choice, repeat);
  }
  else
  {
    for (int i = 0; i < repeat && ok; i++)
    {
      if (!m_Macros.Expand(this, choice))
      {
        ok = false;
      }
    }
  }
    
  m_STC = stc;

  if (ok)
  {
    m_Frame->StatusText(m_Macros.GetMacro(), "PaneMacro");
  }
  
  return ok;
}

void wxExEx::MacroStartRecording(const wxString& macro)
{
  if (!m_IsActive)
  {
    return;
  }
  
  wxString choice(macro);
  
  if (choice.empty())
  {
    wxTextEntryDialog dlg(m_STC,
      _("Input") + ":",
      _("Enter Macro"),
      m_Macros.GetMacro());
  
    wxTextValidator validator(wxFILTER_ALPHANUMERIC);
    dlg.SetTextValidator(validator);
  
    if (dlg.ShowModal() != wxID_OK)
    {
      return;
    }
    
    choice = dlg.GetValue();
  }
  
  m_Macros.StartRecording(choice);
}

bool wxExEx::MarkerAdd(const wxUniChar& marker, int line)
{
  MarkerDelete(marker);
  
  const int lin = (line == -1 ? m_STC->GetCurrentLine(): line);
  
  const int id = m_STC->MarkerAdd(
    lin, 
    m_MarkerSymbol.GetNo());
    
  if (id == -1)
  {
    wxLogError("Could not add marker: %c to line: %d",
      marker, lin);
    return false;  
  }
    
  m_Markers[marker] = id;
  
  return true;
}  

bool wxExEx::MarkerDelete(const wxUniChar& marker)
{
  const auto it = m_Markers.find(marker);

  if (it != m_Markers.end())
  {
    m_STC->MarkerDeleteHandle(it->second);
    m_Markers.erase(it);
    return true;
  }
  
  return false;
}

bool wxExEx::MarkerGoto(const wxUniChar& marker)
{
  const int line = MarkerLine(marker);
  
  if (line != -1)
  {
    m_STC->GotoLineAndSelect(line + 1);
    return true;
  }
  
  return false;
}

int wxExEx::MarkerLine(const wxUniChar& marker) const
{
  if (marker == '<')
  {
    if (m_STC->GetSelectedText().empty())
    {
      return -1;
    }
    
    return m_STC->LineFromPosition(m_STC->GetSelectionStart());
  }
  else if (marker == '>')
  {
    if (m_STC->GetSelectedText().empty())
    {
      return -1;
    }
  
    return m_STC->LineFromPosition(m_STC->GetSelectionEnd());
  }
  else
  {
    const auto it = m_Markers.find(marker);

    if (it != m_Markers.end())
    {
      return m_STC->MarkerLineFromHandle(it->second);
    }
    else
    {
      wxBell();
      wxLogStatus(_("Undefined marker: %c"), marker);
      return -1;
    }
  }
}

void wxExEx::Print(const wxString& text)
{
  ShowDialog("Print", text);
}
  
void wxExEx::SetLastCommand(
  const std::string& command,
  bool always)
{
  // First test on '.' and ';' these should never be the last command,
  // even if always were true.
  // Also, undo or placing a marker should not be a last command.
  if (
    command.empty() ||
    command[0] == '.' || command[0] == ';' || 
    command[0] == 'u' || wxString(command).Matches("m?"))
  {
    return;
  }
  
  if (
      always || 
      command == "~" || 
      ( command.size() > 2 && command.front() == ':' && 
        !wxString(command).StartsWith(":ve") &&
        !wxString(command).StartsWith(":help") &&
        !wxString(command).StartsWith(":new")
      ) ||
    ( command.size() > 1 && command.front() != ':' && command.front() != '\t'))
  {
    m_LastCommand = command;
  }
}
 
void wxExEx::SetRegistersDelete(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  for (int i = 9; i >= 2; i--)
  {
    const std::string value(m_Macros.GetRegister(wxUniChar(48 + i - 1)));
    
    if (!value.empty())
    {
      m_Macros.SetRegister(wxUniChar(48 + i), value);
    }
  }
  
  m_Macros.SetRegister('1', value);
}
  
void wxExEx::SetRegisterInsert(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  m_Macros.SetRegister('.', value);
}

void wxExEx::SetRegisterYank(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  m_Macros.SetRegister('0', value);
}

void wxExEx::ShowDialog(const wxString& title, const wxString& text, bool prop_lexer)
{
  if (m_Dialog == nullptr)
  {
    m_Dialog = new wxExSTCEntryDialog(
      wxTheApp->GetTopWindow(),
      title, 
      text,
      wxEmptyString,
      wxOK);
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
  
  m_Dialog->GetSTC()->GetLexer().Set(prop_lexer ? wxExLexer("props"): m_STC->GetLexer());
  m_Dialog->Show();
}

void wxExEx::Yank(bool show_message)
{
  const std::string range(GetSelectedText());
  
  if (range.empty())
  {
    return;
  }
  
  if (GetRegister())
  {
    m_Macros.SetRegister(GetRegister(), range);
  }
  else
  {
    SetRegisterYank(range);
  }
  
  const int lines = wxExGetNumberOfLines(range);
  
  if (lines >= 3 && show_message)
  {
    m_Frame->ShowExMessage(wxString::Format(_("%d lines yanked"), lines - 1));
  }
}
#endif // wxUSE_GUI
