////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wxExEx
//            http://pubs.opengroup.org/onlinepubs/9699919799/
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/cmdline.h>
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/ex.h>
#include <wx/extension/address.h>
#include <wx/extension/addressrange.h>
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

class wxExCmdLineParser : public wxCmdLineParser
{
  public:
    // Contructor.
    wxExCmdLineParser(const wxString& cmdline) 
      : wxCmdLineParser(cmdline) {;};

    // Adds a negatable switch.
    void AddNegatableSwitch(const wxString& name, const wxString& desc) {
      AddSwitch(name, wxEmptyString, desc, wxCMD_LINE_SWITCH_NEGATABLE);};
    
    // Calls process if switch found.
    void Switch(const wxString& name, std::function<void(bool)> process ) const {
      if (Found(name))
        process((FoundSwitch(name) == wxCMD_SWITCH_ON));};
};

wxExSTCEntryDialog* wxExEx::m_Dialog = NULL;
wxExViMacros wxExEx::m_Macros;
std::string wxExEx::m_LastCommand;

wxExEx::wxExEx(wxExSTC* stc)
  : m_STC(stc)
  , m_Frame(wxDynamicCast(wxTheApp->GetTopWindow(), wxExManagedFrame))
  , m_IsActive(true)
  , m_SearchFlags(wxSTC_FIND_REGEXP)
  , m_MarkerSymbol(0, -1)
  , m_Register(0)
{
  wxASSERT(m_Frame != NULL);
  
  if (wxExFindReplaceData::Get()->MatchCase())
  {
    m_SearchFlags = m_SearchFlags | wxSTC_FIND_MATCHCASE;
  }
}

wxExEx::~wxExEx()
{
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

bool wxExEx::Command(const std::string& command)
{
  if (!m_IsActive || command.empty() || command.front() != ':')
  {
    return false;
  }
  
  bool result = true;

  if (command == ":" || command == ":'<,'>")
  {
    m_Frame->GetExCommand(this, command);
    return true;
  }
  else if (command.compare(0, 3, ":ab") == 0)
  {
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
        output += it.first + " " + it.second + "\n";
      }
      
      ShowDialog("Abbreviations", output);
    }
  }
  else if (command == ":close")
  {
    POST_COMMAND( wxID_CLOSE )
  }
  else if (command.compare(0, 2, ":e") == 0)
  {
    POST_COMMAND( wxID_OPEN )
  }
  else if (command.compare(0, 5, ":grep") == 0) // before :g
  {
    POST_COMMAND( ID_TOOL_REPORT_FIND )
  }
  else if (command.compare(0, 5, ":help") == 0)
  {
    POST_COMMAND( wxID_HELP )
  }
  else if (command == ":n")
  {
    wxExSTC* stc = m_Frame->ExecExCommand(ID_EDIT_NEXT);
    
    if (stc != NULL)
    {
      if (m_Macros.IsPlayback())
      {
        m_STC = stc;
      }
    }
    else
    {
      result = false;
    }
  }
  else if (command.compare(0, 4, ":new") == 0)
  {
    POST_COMMAND( wxID_NEW )
  }
  else if (command == ":prev")
  {
    wxExSTC* stc = m_Frame->ExecExCommand(ID_EDIT_PREVIOUS);
    
    if (stc != NULL)
    {
      if (m_Macros.IsPlayback())
      {
        m_STC = stc;
      }
    }
    else
    {
      result = false;
    }
  }
  else if (command == ":print")
  {
    if (command.find(" ") == std::string::npos)
    {
      m_STC->Print();
    }
    else
    {
      m_STC->Print(false); // no prompt
    }
  }
  else if (command == ":q")
  {
    POST_CLOSE( wxEVT_CLOSE_WINDOW, true )
  }
  else if (command == ":q!")
  {
    POST_CLOSE( wxEVT_CLOSE_WINDOW, false )
  }
  else if (command == ":reg")
  {
    wxString output;
    
    for (const auto& it : m_Macros.GetRegisters())
    {
      output += it + "\n";
    }
  
    output += "%: " + m_STC->GetFileName().GetFullName() + "\n";
    
    ShowDialog("Registers", output);
  }
  else if (command.compare(0, 4, ":sed") == 0)
  {
    POST_COMMAND( ID_TOOL_REPORT_REPLACE )
  }
  else if (command.compare(0, 4, ":set") == 0)
  {
    if (command.find(" ") == std::string::npos)
    {
      POST_COMMAND( wxID_PREFERENCES )
    }
    else
    {
      result = CommandSet(wxString(command.substr(4)).Trim(false));
    }
  }
  else if (command.compare(0, 7, ":syntax") == 0)
  {
    if (wxString(command).EndsWith("on"))
    {
      wxExLexers::Get()->RestoreTheme();
      m_STC->SetLexer(m_STC->GetFileName().GetLexer().GetDisplayLexer(), true); // allow folding
    }
    else if (wxString(command).EndsWith("off"))
    {
      m_STC->ResetLexer();
      wxExLexers::Get()->SetThemeNone();
    }
    else
    {
      result = false;
    }

    m_Frame->StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
  }
  else if (command.compare(0, 4, ":una") == 0)
  {
    wxStringTokenizer tkz(command, " ");
    
    if (tkz.CountTokens() >= 2)
    {
      tkz.GetNextToken(); // skip
      const wxString ab(tkz.GetNextToken());
      m_Macros.SetAbbreviation(ab, "");
    }
  }
  else if (command.compare(0, 3, ":ve") == 0)
  {
    ShowDialog("Version", wxExGetVersionInfo().GetVersionOnlyString());
  }
  else if (command == ":x")
  {
    POST_COMMAND( wxID_SAVE )
    POST_CLOSE( wxEVT_CLOSE_WINDOW, true )
  }
  else if (CommandAddress(command.substr(1)))
  {
    // do nothing
  }
  else if (wxExAddress(this, command.substr(1)).GetLine() > 0)
  {
    m_STC->GotoLineAndSelect(wxExAddress(this, command.substr(1)).GetLine());
  }
  else
  {
    result = false;
  }

  if (result)
  {  
    SetLastCommand(command);
    m_Macros.Record(command);
  }
  else
  {
    wxBell();
  }
  
  return result;
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
    const std::string addr("[0-9\\.\\$\\+\\-]+");
    const std::string addrs("[\\?/].*?[\\?/]"); // non-greedy!
    const std::string addrm("'[a-z]");
    const std::string cmd_group1("([aikr=]|pu)(.*)");
    const std::string cmd_group2("([cdgjmpsStywy<>\\!&~])(.*)");
    std::vector <wxString> v;
    
    if (
      // a % address range
      wxExMatch("^%" + cmd_group2, rest.ToStdString(), v) == 2 ||
      // a search address range
      wxExMatch("^(" + addrs + ")(," + addrs + ")" + cmd_group2, rest.ToStdString(), v) == 4 ||
      // a search address =
      wxExMatch("^(" + addrs + ")(=)(.*)", rest.ToStdString(), v) == 3 ||
      // an address range containing markers
      wxExMatch("^(" + addrm + ")(," + addrm + ")?" + cmd_group2, rest.ToStdString(), v) == 4 ||
      // an addr1 range
      wxExMatch("^(" + addr + ")?" + cmd_group1, rest.ToStdString(), v) == 3 ||
      // an addr2 range
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

      if (!wxExReplaceMarkers(range_str, this))
      {
        return false;
      }
    }
    else 
    {
      return false;
    }
    
    if (range_str.empty() && cmd != '!') 
    {
      range_str = (cmd == "g" || cmd == 'w' ? "%": ".");
    }
  }
  
  if (addr1)
  {
    wxExAddress addr(this, range_str);
    
    switch ((int)cmd.GetChar(0))
    {
    case 0: return false; break;
    case 'a': return addr.Append(rest); break;
    case 'i': return addr.Insert(rest); break;
    case 'k': return addr.MarkerAdd(rest.GetChar(0)); break;
    case 'p': return addr.Put(rest.GetChar(0)); break;
    case 'r': return addr.Read(rest); break;
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
    case 'g': return range.Global(rest); break;
    case 'j': return range.Join(); break;
    case 'm': return range.Move(wxExAddress(this, rest)); break;
    case 'p': return range.Print(rest); break;
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
    case '!': return range.Filter(rest); break;
    default:
      wxLogStatus("Unknown range command: %s", cmd);
      return false;
    }
  }
}

bool wxExEx::CommandSet(const wxString& arg)
{
  wxString text(arg);
  
  if (!text.Contains("/") && (text.Contains("=") || !text.Contains("-")))
  {
    // Convert modeline to commandline arg (add - to each group, remove all =).
    // ts=120 ac ic sy=cpp -> -ts 120 -ac -ic -sy cpp
    std::regex re("[0-9a-z=]+");
    text = std::regex_replace(text.ToStdString(), re, "-&", std::regex_constants::format_sed);
    text.Replace("=", " ");
    text.Replace("!", "-"); // change negatable char
  }
  
  wxExCmdLineParser cl(text);
  cl.AddSwitch("h", wxEmptyString, "help", wxCMD_LINE_OPTION_HELP);
  cl.AddOption("ec", wxEmptyString, "Edge Column", wxCMD_LINE_VAL_NUMBER);
  cl.AddOption("in", wxEmptyString, "INdentation", wxCMD_LINE_VAL_NUMBER);
  cl.AddOption("sy", wxEmptyString, "SYntax (off)", wxCMD_LINE_VAL_STRING);
  cl.AddOption("ts", wxEmptyString, "Tab Stop", wxCMD_LINE_VAL_NUMBER);
  cl.AddNegatableSwitch("ai", "Auto Indent");
  cl.AddNegatableSwitch("ac", "Auto Complete");
  cl.AddNegatableSwitch("el", "Edge Line");
  cl.AddNegatableSwitch("ic", "Ignore Case");
  cl.AddNegatableSwitch("mw", "Match Words");
  cl.AddNegatableSwitch("nu", "show LineNumbers");
  cl.AddNegatableSwitch("re", "Regular Expression");
  cl.AddNegatableSwitch("ut", "Use Tabs");
  cl.AddNegatableSwitch("wl", "Wrap Line");
  cl.AddNegatableSwitch("ws", "show WhiteSpace");
  
  switch (cl.Parse())
  {
    case -1: return true; // help
    case 0: break; // ok 
    default: return false; // error
  }
  
  if (cl.Found("ec"))
  {
    long val;
    cl.Found("ec", &val);
    m_STC->SetEdgeColumn(val);
    wxConfigBase::Get()->Write(_("Edge column"), val);
  }
  if (cl.Found("in"))
  {
    long val;
    cl.Found("in", &val);
    m_STC->SetIndent(val);
    wxConfigBase::Get()->Write(_("Indent"), val);
  }
  if (cl.Found("sy"))
  {
    wxString val;
    cl.Found("sy", &val);
    if (val != "off") m_STC->SetLexer(val, true); // allow folding
    else              m_STC->ResetLexer();
  }
  if (cl.Found("ts"))
  {
    long val;
    cl.Found("ts", &val);
    m_STC->SetTabWidth(val);
    wxConfigBase::Get()->Write(_("Tab width"), val);
  }
  
  cl.Switch("ai", [](bool on){wxConfigBase::Get()->Write(_("Auto indent"), on ? 2: 0);});
  cl.Switch("ac", [](bool on){wxConfigBase::Get()->Write(_("Auto complete"), on);});
  cl.Switch("ic", [&](bool on){
    if (!on) m_SearchFlags |= wxSTC_FIND_MATCHCASE;
    else     m_SearchFlags &= ~wxSTC_FIND_MATCHCASE;
    wxExFindReplaceData::Get()->SetMatchCase(!on);});
  cl.Switch("el", [&](bool on){
    m_STC->SetEdgeMode(on ? wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);
    wxConfigBase::Get()->Write(_("Edge line"), on ? wxSTC_EDGE_LINE: wxSTC_EDGE_NONE);});
  cl.Switch("mw", [&](bool on){
    if (on) m_SearchFlags |= wxSTC_FIND_WHOLEWORD;
    else    m_SearchFlags &= ~wxSTC_FIND_WHOLEWORD;
    wxExFindReplaceData::Get()->SetMatchWord(on);});
  cl.Switch("nu", [&](bool on){
    m_STC->ShowLineNumbers(on);
    wxConfigBase::Get()->Write(_("Line numbers"), on);});
  cl.Switch("re", [&](bool on){
    if (on) m_SearchFlags |= wxSTC_FIND_REGEXP;
    else    m_SearchFlags &= ~wxSTC_FIND_REGEXP;
    wxExFindReplaceData::Get()->SetUseRegEx(on);});
  cl.Switch("ut", [&](bool on){
    m_STC->SetUseTabs(on);
    wxConfigBase::Get()->Write(_("Use tabs"), on);});
  cl.Switch("wl", [&](bool on){
    m_STC->SetWrapMode(on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);
    wxConfigBase::Get()->Write(_("Wrap line"), on ? wxSTC_WRAP_CHAR: wxSTC_WRAP_NONE);});
  cl.Switch("ws", [&](bool on){
    m_STC->SetViewEOL(on);
    m_STC->SetViewWhiteSpace(on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);
    wxConfigBase::Get()->Write(_("Whitespace"), on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);});
  
  return true;
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
    GetFrame()->ShowExMessage(wxString::Format(_("%d fewer lines"), lines - 1));
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

void wxExEx::MacroRecord(const std::string& text)
{
  m_Macros.Record(text);
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
      ( command.size() > 1 && command.front() == ':' && 
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

void wxExEx::ShowDialog(const wxString& title, const wxString& text)
{
  if (m_Dialog == NULL)
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
    GetFrame()->ShowExMessage(wxString::Format(_("%d lines yanked"), lines - 1));
  }
}

#endif // wxUSE_GUI
