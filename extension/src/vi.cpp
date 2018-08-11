////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <regex>
#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/vi.h>
#include <wx/extension/addressrange.h>
#include <wx/extension/ctags.h>
#include <wx/extension/frd.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>
#include <wx/extension/vi-macros.h>
#include <wx/extension/vi-macros-mode.h>
#include <easylogging++.h>

// compares two strings in compile time constant fashion
constexpr int c_strcmp( char const* lhs, char const* rhs )
{
  return (('\0' == lhs[0]) && ('\0' == rhs[0])) ? 0
    :  (lhs[0] != rhs[0]) ? (lhs[0] - rhs[0])
    : c_strcmp( lhs+1, rhs+1 );
};

#define MOTION(SCOPE, DIRECTION, COND, WRAP)                           \
{                                                                      \
  for (auto i = 0; i < m_Count; i++)                                   \
  {                                                                    \
    switch (Mode().Get())                                              \
    {                                                                  \
      case wxExViModes::NORMAL:                                        \
      case wxExViModes::INSERT:                                        \
        if (WRAP && c_strcmp((#SCOPE), "Line") ==0)                    \
        {                                                              \
          if (c_strcmp((#DIRECTION), "Down") == 0)                     \
            GetSTC()->LineEnd();                                       \
          else                                                         \
            GetSTC()->Home();                                          \
        }                                                              \
        GetSTC()->SCOPE##DIRECTION();                                  \
        break;                                                         \
      case wxExViModes::VISUAL: GetSTC()->SCOPE##DIRECTION##Extend();  \
        break;                                                         \
      case wxExViModes::VISUAL_LINE:                                   \
        if (c_strcmp((#SCOPE), "Char") != 0 &&                         \
            c_strcmp((#SCOPE), "Word") != 0)                           \
          GetSTC()->SCOPE##DIRECTION##Extend();                        \
        break;                                                         \
      case wxExViModes::VISUAL_RECT: GetSTC()->SCOPE##DIRECTION##RectExtend(); \
        break;                                                         \
    }                                                                  \
  }                                                                    \
  if (c_strcmp((#SCOPE), "Line") == 0)                                 \
  {                                                                    \
    switch (Mode().Get())                                              \
    {                                                                  \
      case wxExViModes::NORMAL:                                        \
      case wxExViModes::INSERT:                                        \
        if ((COND) &&                                                  \
          GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) !=            \
          GetSTC()->GetLineIndentation(GetSTC()->GetCurrentLine()))    \
          GetSTC()->VCHome(); break;                                   \
      case wxExViModes::VISUAL:                                        \
        if (COND) GetSTC()->VCHomeExtend(); break;                     \
    }                                                                  \
  }                                                                    \
  return 1;                                                            \
}                                                                      \

#define REPEAT(TEXT)                 \
{                                    \
  for (auto i = 0; i < m_Count; i++) \
  {                                  \
    TEXT;                            \
  }                                  \
}                                    \

#define REPEAT_WITH_UNDO(TEXT)       \
{                                    \
  GetSTC()->BeginUndoAction();       \
  REPEAT(TEXT);                      \
  GetSTC()->EndUndoAction();         \
}

char ConvertKeyEvent(const wxKeyEvent& event)
{
  if (event.GetKeyCode() == WXK_BACK) return WXK_BACK;

  char c = event.GetUnicodeKey();
  
  if (c == WXK_NONE)
  {
    switch (event.GetKeyCode())
    {
      case WXK_LEFT:     c = 'h'; break;
      case WXK_DOWN:     c = 'j'; break;
      case WXK_UP:       c = 'k'; break;
      case WXK_RIGHT:    c = 'l'; break;
      case WXK_DELETE:   c = 'x'; break;
      case WXK_PAGEUP:   c = WXK_CONTROL_B; break;
      case WXK_PAGEDOWN: c = WXK_CONTROL_F; break;
      case WXK_NUMPAD_ENTER: c = '\r'; break;
      default: c = event.GetKeyCode();
    }
  }

  return c;
}

// no auto, does not compile under MSW
bool DeleteRange(wxExVi* vi, int start, int end)
{
  if (!vi->GetSTC()->GetReadOnly())
  {
    const auto first = (start < end ? start: end);
    const auto last = (start < end ? end: start);
  
    if (!vi->GetSTC()->HexMode())
    {
      const wxCharBuffer b(vi->GetSTC()->GetTextRangeRaw(first, last));
    
      vi->GetMacros().SetRegister(
        vi->GetRegister() ? vi->GetRegister(): '0', 
        std::string(b.data(), b.length()));
      
      vi->GetSTC()->DeleteRange(first, last - first);
    }
    else
    {
      vi->GetSTC()->GetHexMode().Delete(last - first, first);
    }
  }
  
  return true;
}

wxExVi::wxExVi(wxExSTC* stc)
  : wxExEx(stc)
  , m_Mode(this, 
     // insert mode process
     [=](const std::string& command) {
        if (!m_Dot)
        {
          m_InsertText.clear();
        }
        GetSTC()->BeginUndoAction();},
     // back to normal mode process
     [=]() {
        if (!m_Dot)
        {
          const std::string ESC("\x1b");
          const std::string lc(m_InsertCommand + GetRegisterInsert());
          SetLastCommand(lc + ESC);
          // Record it (if recording is on).
          GetMacros().Record(lc);
          GetMacros().Record(ESC);
        }
        m_Command.clear();
        m_InsertCommand.clear();
        GetSTC()->AutoComplete().Reset();
        GetSTC()->EndUndoAction();})
  , m_LastCommands {
    {"!", "<", ">", 
     "A", "C", "D", "I", "J", "O", "P", "R", "S", "X", "Y", 
     "a", "c", "d", "i", "o", "p", "r", "s", "x", "y", "~"}}
  , m_MotionCommands {
    {"h", [&](const std::string& command){
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0) 
        MOTION(Char, Left, false, false); 
      return 1;}},
    {"j", [&](const std::string& command){MOTION(Line, Down, false, false);}},
    {"k", [&](const std::string& command){MOTION(Line, Up, false, false);}},
    {"l ", [&](const std::string& command){
      if (command == "l" && GetSTC()->GetCurrentPos() >= 
          GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine())) return 1; 
      MOTION(Char, Right, false, false);}},
    {"b", [&](const std::string& command){MOTION(Word, Left, false, false);}},
    {"eE", [&](const std::string& command){MOTION(Word, RightEnd, false, false);}},
    {"w", [&](const std::string& command){MOTION(Word, Right, false, false);}},
    {"fFtT,;", [&](const std::string& command){
      if (command.empty()) return (size_t)0;
      char c; // char to find
      if (command.size() == 1)
      {
        if (command[0] == ';' || command[0] == ',')
        {
          if (m_LastFindCharCommand.empty()) return (size_t)0;
          c = m_LastFindCharCommand.back();
        }
        else
        {
          return (size_t)0;
        }
      }
      else
      {
        c = command[1];
      }
      char d; // char specifying direction
      switch (command[0])
      {
        case ';': d = m_LastFindCharCommand.front(); break;
        case ',': 
          d = m_LastFindCharCommand.front();
          if (islower(d)) d = toupper(d);
          else d = tolower(d);
          break;
        default: 
          if (command.size() > 1) d = command.front();
          else d = m_LastFindCharCommand.front();
      }
      REPEAT(
        if (!GetSTC()->FindNext(
          std::string(1, c), 
          GetSearchFlags() & ~wxSTC_FIND_REGEXP & ~wxSTC_FIND_WHOLEWORD, 
          islower(d) > 0))
        {
          return m_Command.clear();
        });
      if (command[0] != ',' && command[0] != ';')
      {
        m_LastFindCharCommand = command;
      }
      if (tolower(d) == 't') 
      {
        GetSTC()->CharLeft();
      }
      return command.size();}},
    {"nN", [&](const std::string& command){REPEAT(
      if (const std::string find(GetSTC()->GetMarginTextClick() > 0 ?
        wxExListFromConfig("exmargin").front():
        wxExFindReplaceData::Get()->GetFindString());
        !GetSTC()->FindNext(
        find,
        GetSearchFlags(), 
        command == "n" ? m_SearchForward: !m_SearchForward))
      {
        return m_Command.clear();
      });
      return (size_t)1;}},
    {"G", [&](const std::string& command){
      if (m_Count == 1)
      {
        m_Mode.Visual() ?
          GetSTC()->DocumentEndExtend():
          GetSTC()->DocumentEnd();
      }
      else
      {
        (void)wxExSTCData(GetSTC()).Control(
           wxExControlData().Line(m_Count)).Inject();
      }
      return 1;}},
    {"H", [&](const std::string& command){
      GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine());
      return 1;}},
    {"L", [&](const std::string& command){
      GetSTC()->GotoLine(
        GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() - 1);
      return 1;}},
    {"M", [&](const std::string& command){
      GetSTC()->GotoLine(
        GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() / 2);
      return 1;}},
    {"/?", [&](const std::string& command){
      m_SearchForward = command.front() == '/';

      if (command.size() > 1)
      {
        if (command[1] == WXK_CONTROL_R)
        {
          if (command.size() < 3)
          {
            return (size_t)0;
          }

          if (!GetSTC()->FindNext(
            GetMacros().GetRegister(command[2]),
            GetSearchFlags(),
            m_SearchForward)) return (size_t)0;
          wxExFindReplaceData::Get()->SetFindString(
            GetMacros().GetRegister(command[2]));
          return command.size();
        }
        
        // This is a previous entered command.
        if (!GetSTC()->FindNext(
          command.substr(1),
          GetSearchFlags(),
          m_SearchForward)) return (size_t)0;
        if (GetSTC()->GetMarginTextClick() == -1)
          wxExFindReplaceData::Get()->SetFindString(command.substr(1));
        return command.size();
      }
      else
      {
        return GetFrame()->GetExCommand(this, 
          command + (Mode().Visual() ? "'<,'>": "")) ? command.size(): (size_t)0;
      }}},
    {"\'", [&](const std::string& command){
      if (wxExOneLetterAfter("'", command)) 
      {
        const auto pos = GetSTC()->GetCurrentPos();
        MarkerGoto(command.back()); 
        VisualExtend(pos, GetSTC()->GetCurrentPos());
        return 2;
      } 
      return 0;}},
    {"0^", [&](const std::string& command){MOTION(Line, Home, false, false);}},
    {"[]", [&](const std::string& command){REPEAT(
      if (!GetSTC()->FindNext("{", GetSearchFlags(), command == "]"))
      {
        return m_Command.clear();
      })
      return (size_t)1;}},
    {"({", [&](const std::string& command){MOTION(Para, Up,   false, false);}},
    {")}", [&](const std::string& command){MOTION(Para, Down, false, false);}},
    {"+", [&](const std::string& command){MOTION(Line, Down, true,  true); }},
    {"|", [&](const std::string& command){
      GetSTC()->GotoPos(GetSTC()->PositionFromLine(
        GetSTC()->GetCurrentLine()) + m_Count - 1);
      return 1;}},
    {"-", [&](const std::string& command){MOTION(Line, Up,  true, true);}},
    {"$", [&](const std::string& command){MOTION(Line, End, false, false);}},
    {"%", [&](const std::string& command){
      auto pos = GetSTC()->GetCurrentPos();
      auto brace_match = GetSTC()->BraceMatch(pos);
      if (brace_match == wxSTC_INVALID_POSITION)
      {
        brace_match = GetSTC()->BraceMatch(--pos);
      }
      if (brace_match != wxSTC_INVALID_POSITION)
      {
        GetSTC()->GotoPos(brace_match);
        VisualExtend(pos, brace_match + 1);
      }
      return 1;}},
    {"!", [&](const std::string& command){
      if (command.size() > 1)
      {
        return wxExEx::Command(":" + command);
      }
      else
      {
        return GetFrame()->GetExCommand(this, command);
      }}},
    {"\r_", [&](const std::string& command){
      GetSTC()->Home();
      MOTION(Line, Down, false, false);}},
    {"\x02", [&](const std::string& command){MOTION(Page, Up,         false, false);}},
    {"\x06", [&](const std::string& command){MOTION(Page, Down,       false, false);}},
    {"\x10", [&](const std::string& command){MOTION(Line, ScrollUp,   false, false);}},
    {"\x11", [&](const std::string& command){MOTION(Line, ScrollDown, false, false);}}}
  , m_OtherCommands {
    {"m", [&](const std::string& command){
      if (wxExOneLetterAfter("m", command))
      {
        MarkerAdd(command.back());
        return 2;
      }
      return 0;}},
    {"p", [&](const std::string& command){Put(true);
      return 1;}},
    {"q", [&](const std::string& command){
      return GetMacros().Mode()->Transition(command, this, false, m_Count);}},
    {"@", [&](const std::string& command){
      return GetMacros().Mode()->Transition(command, this, false, m_Count);}},
    {"r", [&](const std::string& command){
      if (command.size() > 1 &&
        !GetSTC()->GetReadOnly())
      {
        if (GetSTC()->HexMode())
        {
          if (!GetSTC()->GetHexMode().Replace(command.back()))
          {
            return m_Command.clear();
          }
        }
        else
        {
          GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
          GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + m_Count);
          GetSTC()->ReplaceTarget(wxString(command.back(), m_Count));
        }
        return (size_t)2;
      }
      return (size_t)0;}},
    {"u", [&](const std::string& command){
      if (GetSTC()->CanUndo()) GetSTC()->Undo();
      else 
      {
        if (wxConfigBase::Get()->ReadLong(_("Error bells"), 1))
        {
          wxBell();
        }
      }
      return 1;}},
    {"x", [&](const std::string& command){
      return DeleteRange(
        this, 
        GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + m_Count) ? 1: 0;}},
    {"J", [&](const std::string& command){
        wxExAddressRange(this, m_Count).Join(); return 1;}},
    {"P", [&](const std::string& command){Put(false);return 1;}},
    // tag commands ->
    {"Q", [&](const std::string& command){
      GetFrame()->SaveCurrentPage("ctags");
      if (GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) == 0 ?
          GetCTags()->Find(std::string()):
          GetCTags()->Find(GetSTC()->GetWordAtPos(GetSTC()->GetCurrentPos()));
      }
      else
      {
        GetCTags()->Find(GetSTC()->GetSelectedText().ToStdString());
      }
      return 1;}},
    {"S", [&](const std::string& command){
      GetFrame()->RestorePage("ctags");
      return 1;}},
    {"B", [&](const std::string& command){
      GetFrame()->SaveCurrentPage("ctags");
      GetCTags()->Previous();
      return 1;}},
    {"W", [&](const std::string& command){
      GetFrame()->SaveCurrentPage("ctags");
      GetCTags()->Next();
      return 1;}},
    // <- tag commands
    {"X", [&](const std::string& command){
      return DeleteRange(this, 
        GetSTC()->GetCurrentPos() - m_Count, GetSTC()->GetCurrentPos()) ? 1: 0;}},
    {"dd", [&](const std::string& command){
      wxExAddressRange(this, m_Count).Delete();
      return command.size();}},
    {"dgg", [&](const std::string& command){
      return DeleteRange(this, 0, 
        GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine())) ? 3: 0;}},
    {"gg", [&](const std::string& command){
      m_Mode.Visual() ?
        GetSTC()->DocumentStartExtend():
        GetSTC()->DocumentStart(); 
      return 2;}},
    {"yy", [&](const std::string& command){
      wxExAddressRange(this, m_Count).Yank();
      return command.size();}},
    {"ZZ", [&](const std::string& command){
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCloseEvent(wxEVT_CLOSE_WINDOW)); 
      return 2;}},
    {"z", [&](const std::string& command){
      if (command.size() <= 1) return (size_t)0;
      const auto level = GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine());
      const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
        GetSTC()->GetCurrentLine(): 
        GetSTC()->GetFoldParent(GetSTC()->GetCurrentLine());
      switch (command[1])
      {
        case 'c':
        case 'o':
          if (GetSTC()->GetFoldExpanded(line_to_fold) && command == "zc")
            GetSTC()->ToggleFold(line_to_fold);
          else if (!GetSTC()->GetFoldExpanded(line_to_fold) && command == "zo")
            GetSTC()->ToggleFold(line_to_fold);
          break;
        case 'f':
          GetSTC()->GetLexer().SetProperty("fold", "1");
          GetSTC()->GetLexer().Apply();
          GetSTC()->Fold(true); break;
        case 'E':
          GetSTC()->GetLexer().SetProperty("fold", "0");
          GetSTC()->GetLexer().Apply();
          GetSTC()->Fold(false); break;
        case 'M':
          GetSTC()->Fold(true);
          break;
        case 'R':
          for (int i = 0; i < GetSTC()->GetLineCount(); i++) 
            GetSTC()->EnsureVisible(i);
          break;
      }; 
      return command.size();}},
    {".", [&](const std::string& command){
      if (m_LastCommand.empty())
      {
        return 1;
      }
      m_Dot = true;
      const auto count = m_Count;
      GetSTC()->BeginUndoAction();
      for (int i = 0; i < count; i++)
      {
        Command(m_LastCommand);
      }
      GetSTC()->EndUndoAction();
      m_Dot = false;
      return 1;}},
    {"~", [&](const std::string& command){
      if (GetSTC()->GetLength() == 0 || 
          GetSTC()->GetReadOnly() || 
          GetSTC()->HexMode()) return 0;
      REPEAT_WITH_UNDO(
        if (GetSTC()->GetCurrentPos() == GetSTC()->GetLength()) return 0;
        wxString text(GetSTC()->GetTextRange(
            GetSTC()->GetCurrentPos(), 
            GetSTC()->GetCurrentPos() + 1));
        if (text.empty()) return 0;
        islower(text[0]) ? text.UpperCase(): text.LowerCase();
        GetSTC()->wxStyledTextCtrl::Replace(
          GetSTC()->GetCurrentPos(), 
          GetSTC()->GetCurrentPos() + 1, 
          text);
        GetSTC()->CharRight());
      return 1;}},
    {"><", [&](const std::string& command){
      switch (Mode().Get())
      {
        case wxExViModes::NORMAL:
          wxExAddressRange(this, m_Count).Indent(command == ">"); break;
        case wxExViModes::VISUAL:
        case wxExViModes::VISUAL_LINE:
        case wxExViModes::VISUAL_RECT:
          wxExAddressRange(this, "'<,'>").Indent(command == ">"); break;
      }
      return 1;}},
    {"*#U", [&](const std::string& command){
      const auto start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
      const auto end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos(), true);
      const std::string word(GetSTC()->GetSelectedText().empty() ?
        GetSTC()->GetTextRange(start, end).ToStdString():
        GetSTC()->GetSelectedText().ToStdString());
      if (!word.empty())
      {
        if (command == "U")
        {
          wxExBrowserSearch(word);
        }
        else
        {
          wxExFindReplaceData::Get()->SetFindString("\\<"+ word + "\\>");
          GetSTC()->FindNext(
            wxExFindReplaceData::Get()->GetFindString(), 
            GetSearchFlags(), 
            command == "*");
        }
      }
      return 1;}},
    {"\t", [&](const std::string& command){
      // just ignore tab, except on first col, then it indents
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) == 0)
      {
        return m_Command.clear();
      }
      return (size_t)1;}},
    // ctrl-e, ctrl-j
    {"\x05\x0a", [&](const std::string& command){REPEAT_WITH_UNDO(
      if (GetSTC()->HexMode()) return 1;
      try 
      {
        const auto start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
        const auto sign = (GetSTC()->GetCharAt(start) == '-' ? 1: 0);
        const auto end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos() + sign, true);
        const std::string word(GetSTC()->GetTextRange(start, end).ToStdString());
        auto number = std::stoi(word, nullptr, 0);
        const auto next = (command == "\x05" ? ++number: --number);
        std::ostringstream format;
        format.fill(' ');
        format.width(end - start);
        if (word.substr(0, 2) == "0x") format << std::hex;
        else if (word[0] == '0') format << std::oct;
        format << std::showbase << next;
        GetSTC()->wxStyledTextCtrl::Replace(start, end, format.str());
      }
      catch (...)
      {
      })
      return 1;}},
    // ctrl-h
    {"\x08", [&](const std::string& command){
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode()) GetSTC()->DeleteBack();
      return command.size();}},
    // ctrl-r
    {"\x12", [&](const std::string& command){
      if (command.size() > 1 &&
        wxExRegAfter(std::string(1, WXK_CONTROL_R), command))
      {
        CommandReg(command[1]);
        return command.size();
      }  
      return (size_t)0;}},
    // delete char
    {"\x7F", [&](const std::string& command){
      return DeleteRange(this, 
        GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + m_Count) ? 1: 0;}}}
{
}

void wxExVi::AddText(const std::string& text)
{
  if (!GetSTC()->GetOvertype())
  {
    GetSTC()->AddTextRaw(text.c_str(), text.size());
  }
  else
  {
    GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
    GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + text.size());
    GetSTC()->ReplaceTarget(text);
  }
}

void wxExVi::AppendInsertCommand(const std::string& s)
{
  m_InsertCommand.append(s);
}

bool wxExVi::Command(const std::string& command)
{
  if (command.empty() || !GetIsActive())
  {
    return false;
  }

  VLOG(9) << "vi command: " << command;

  if (Mode().Visual() && command.find("'<,'>") == std::string::npos &&
    wxExEx::Command(command + "'<,'>"))
  {
    return true;
  }
  else if (command.front() == '=' ||
   (command.size() > 2 && 
    command.find(std::string(1, WXK_CONTROL_R) + "=") == 0))
  {
    CommandCalc(command);
    GetMacros().Record(command);
    return true;
  }
  else if (Mode().Insert())
  {
    return InsertMode(command);
  }
  else if (!m_Dot && command.back() == WXK_ESCAPE)
  {
    m_Mode.Escape();
    m_Command.clear();
    m_InsertCommand.clear();
    return true;
  }

  if (
    auto parse(command); !ParseCommand(parse))
  {
    return false;
  }
  else 
  {
    if (!m_Dot)
    {
      SetLastCommand(command);
    }
        
    if (GetMacros().Mode()->IsRecording() && 
      command[0] != 'q' && command != "/" && command != "?")
    {
      GetMacros().Record(command);
    }  

    return true;
  }
}

void wxExVi::CommandCalc(const std::string& command)
{
  if (const auto [sum, width] = Calculator(
    command.substr(command[0] == '=' ? 1: 2)); !std::isnan(sum))
  {
    if (Mode().Insert())
    {
      if (m_LastCommand.find('c') != std::string::npos)
      {
        GetSTC()->ReplaceSelection(wxEmptyString);
      }
    
      AddText(wxString::Format("%.*f", width, sum).ToStdString());
    }
    else
    {
      const wxString msg(wxString::Format("%.*f", width, sum));
      SetRegisterYank(msg.ToStdString());
      GetFrame()->ShowExMessage(msg.ToStdString());
    }
  }
}

void wxExVi::CommandReg(const char reg)
{
  switch (reg)
  {
    case 0: break;
    // calc register
    case '=': GetFrame()->GetExCommand(this, std::string(1, reg)); break;
    // clipboard register
    case '*': 
      if (Mode().Insert())
      {
        Put(true); 
      }
      break;
    // filename register
    case '%':
      if (Mode().Insert())
      {
        AddText(GetSTC()->GetFileName().GetFullName());
      }
      else
      {
        GetFrame()->ShowExMessage(GetSTC()->GetFileName().Path().string());
        wxExClipboardAdd(GetSTC()->GetFileName().Path().string());
      }
      break;
    default:
      if (!GetMacros().GetRegister(reg).empty())
      {
        if (Mode().Insert())
        {   
          AddText(GetMacros().GetRegister(reg));
          
          if (reg == '.')
          {
            m_InsertText += GetRegisterInsert();
          }
        }
        else
        {
          GetFrame()->ShowExMessage(GetMacros().GetRegister(reg));
        }
      }
      else
      {
        GetFrame()->ShowExMessage("?" + std::string(1, reg));
      }
  }
}

void wxExVi::FilterCount(std::string& command)
{
  std::vector<std::string> v;

  /*
   command: 3w
   -> v has 2 elements
   -> m_Count 3
   -> command w
   */
  if (const auto matches = wxExMatch("^([1-9][0-9]*)(.*)", command, v);
    matches == 2)
  {
    const auto count = std::stoi(v[0]);
    m_Count *= count;
    AppendInsertCommand(v[0]);
    command = v[1];
  }
}

bool wxExVi::InsertMode(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }
  else if (GetSTC()->HexMode())
  {
    if ((int)command.back() == WXK_ESCAPE)
    {
      if (m_Mode.Escape())
      {
        GetSTC()->SetOvertype(false);
      }

      return true;
    }
    else
    {
      return GetSTC()->GetHexMode().Insert(command, GetSTC()->GetCurrentPos());
    }
  }
  // add control chars
  else if (command.size() == 2 && command[1] == 0)
  {
    m_InsertText += std::string(1, command[0]);
    AddText(std::string(1, command[0]));
    return true;
  }

  if (command.find(std::string(1, WXK_CONTROL_R) + "=") != std::string::npos)
  {
    if (
      command.compare(0, 2, std::string(1, WXK_CONTROL_R) + "=") == 0)
    {
      CommandReg(command[1]);
      return true;
    }
    else
    {
      InsertMode(wxExFirstOf(command, 
        std::string(1, WXK_CONTROL_R), 0, FIRST_OF_BEFORE));
      CommandCalc(wxExFirstOf(command, std::string(1, WXK_CONTROL_R)));
      return true;
    }
  }
  else if (command.find(std::string(1, WXK_CONTROL_R)) != std::string::npos)
  {
    if (command.size() < 2)
    {
      return false;
    }
    
    for (wxExTokenizer tkz(command, std::string(1, WXK_CONTROL_R), false); 
      tkz.HasMoreTokens(); )
    {
      const auto token = tkz.GetNextToken();
      const auto rest(tkz.GetString());
      
      if (wxExRegAfter("\x12", "\x12" + rest.substr(0, 1)))
      {
        InsertMode(token);
        CommandReg(rest[0]);
        InsertMode(rest.substr(1));
        return true;
      }  
    }
  }
  
  switch ((int)command.back())
  {
    case WXK_BACK: 
      if (!m_InsertText.empty())
      {
        m_InsertText.pop_back();
      }
      GetSTC()->DeleteBack();
      break;
      
    case WXK_CONTROL_R:
      m_InsertText += command;
      break;
        
    case WXK_DELETE: 
      DeleteRange(this, GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + 1);
      break;
      
    case WXK_ESCAPE:
      // Add extra inserts if necessary.        
      if (!m_InsertText.empty())
      {
        for (auto i = 1; i < m_Count; i++) AddText(m_InsertText); // no REPEAT
        SetRegisterInsert(m_InsertText);
      }
    
      // If we have text to be added.
      if (command.size() > 1)
      { 
        if (const auto rest(command.substr(0, command.size() - 1));
          !GetSTC()->GetSelectedText().empty())
        {
          GetSTC()->ReplaceSelection(rest);
        }
        else
        {
          if (!GetSTC()->GetOvertype())
          {
            REPEAT(AddText(rest));
          }
          else
          {
            wxString text;
            GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
            REPEAT(text += rest;);
            GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + text.size());
            GetSTC()->ReplaceTarget(text);
          }
        }
      }
        
      if (m_Mode.Escape())
      {
        GetSTC()->SetOvertype(false);
      }
      break;

    default: 
      if (m_LastCommand.find('c') != std::string::npos && 
        m_InsertText.empty())
      {
        GetSTC()->ReplaceSelection(wxEmptyString);
      }

      if (
       !m_InsertText.empty() &&
        m_InsertText.back() == wxUniChar(WXK_CONTROL_R))
      {
        GetSTC()->ReplaceSelection(wxEmptyString);
        
        if (command.back() != '.')
        {
          m_InsertText += command;
        }
        else
        {
          m_InsertText.pop_back();
        }
      
        CommandReg(command.back());
        return false;
      }
      else
      {
        if (command.size() == 1 && 
          ((int)command.back() == WXK_RETURN || 
           (int)command.back() == WXK_NUMPAD_ENTER))
        {
          GetSTC()->NewLine();
            
          if (!GetSTC()->AutoCompActive())
          {
            m_InsertText += GetSTC()->GetEOL();
          }
        }
        else
        {
          if (!GetSTC()->GetOvertype())
          {
            InsertModeNormal(command);
          }
          
          if (!m_Dot)
          {
            m_InsertText += command;
          }
        }
      }
  }
  
  return true;
}

void wxExVi::InsertModeNormal(const std::string& text)
{
  if (wxExTokenizer tkz(text, "\r\n", false);
    text.find('\0') == std::string::npos && tkz.HasMoreTokens())
  {
    while (tkz.HasMoreTokens())
    {
      if (auto token(tkz.GetNextToken()); !token.empty())
      {
        if (token.back() == ' ' || token.back() == '\t' || token.back() == ';')
        {
          if (!m_InsertText.empty())
          {
            const auto last(m_InsertText.find_last_of(" ;\t"));
            
            if (const auto& it = GetMacros().GetAbbreviations().find(
              m_InsertText.substr(last + 1));
              it != GetMacros().GetAbbreviations().end())
            {
              m_InsertText.replace(last + 1, it->first.size(), it->second);
              
              const auto pos = GetSTC()->GetCurrentPos();
              const auto match_pos = GetSTC()->FindText(
                pos,
                GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()),
                it->first);
                
              if (match_pos != wxSTC_INVALID_POSITION)
              {
                GetSTC()->SetTargetStart(match_pos);
                GetSTC()->SetTargetEnd(match_pos + it->first.size());
                GetSTC()->ReplaceTarget(it->second);
                GetSTC()->SetCurrentPos(pos + it->second.size() - it->first.size());
              }
            }
          }
          else
          {
            const auto last(token.find_last_of(" ;\t", token.size() - 2));
            const auto word(token.substr(last + 1, token.size() - 2 - last));
            
            if (const auto& it = GetMacros().GetAbbreviations().find(word);
              it != GetMacros().GetAbbreviations().end())
            {
              token.replace(last + 1, it->first.size(), it->second);
            }
          }
        }
        
        GetSTC()->AddText(token);
      }
  
      if (tkz.GetLastDelimiter() != 0)
      {
        GetSTC()->AddText(tkz.GetLastDelimiter());
        GetSTC()->AutoIndentation(tkz.GetLastDelimiter());
      }
    }
  }
  else
  {
    AddText(text);
  }
}

bool wxExVi::MotionCommand(MotionType type, std::string& command)
{
  if (command.empty())
  {
    return false;
  }

  FilterCount(command);

  if (!GetSelectedText().empty()) 
  {
    if (type == MOTION_YANK)
    {
      return wxExAddressRange(this, m_Count).Yank();
    }
    else if (type == MOTION_DELETE)
    {
      return wxExAddressRange(this, m_Count).Delete();
    }
  }

  const auto& it = std::find_if(
    m_MotionCommands.begin(), 
    m_MotionCommands.end(), 
    [&](auto const& e) {
      for (const auto& r : e.first) 
      {
        if (r == command[0]) return true;
      }
      return false;});
  
  if (it == m_MotionCommands.end())
  {
    return false;
  }

  int parsed = 0;
  auto start = GetSTC()->GetCurrentPos();
  
  switch (type)
  {
    case MOTION_CHANGE:
      if (!GetSelectedText().empty())
      {
        GetSTC()->SetCurrentPos(GetSTC()->GetSelectionStart());
        start = GetSTC()->GetCurrentPos();
      }
    
      if (!m_Command.IsHandled() && 
        (parsed = it->second(command)) == 0) 
      {
        m_Mode.Escape();
        return false;
      }
    
      DeleteRange(this, start, GetSTC()->GetCurrentPos());
      break;
    
    case MOTION_DELETE:
      if (GetSTC()->GetReadOnly())
      {
        command.clear();
        return true;
      }
    
      if (!m_Command.IsHandled() && 
          (parsed = it->second(command)) == 0) return false;

      DeleteRange(this, start, GetSTC()->GetCurrentPos());
      break;
    
    case MOTION_NAVIGATE: 
      if (!m_Command.IsHandled() && 
          (parsed = it->second(command)) == 0) return false; 
      break;
    
    case MOTION_YANK:
      if (!Mode().Visual())
      {
        std::string visual("v");
        m_Mode.Transition(visual);
      }
    
      if (!m_Command.IsHandled() && 
          (parsed = it->second(command)) == 0) return false;
    
      if (auto end = GetSTC()->GetCurrentPos(); end - start > 0)
      {
        GetSTC()->CopyRange(start, end - start);
        GetSTC()->SetSelection(start, end);
      }
      else
      {
        // reposition end at start of selection
        if (!GetSTC()->GetSelectedText().empty())
        {
          end = GetSTC()->GetSelectionStart();
        }
        else
        {
          end--;
        }
      
        GetSTC()->CopyRange(end, start - end);
        GetSTC()->SetSelection(end, start);
      }

      m_Mode.Escape();
      
      if (!GetRegister())
      {
        SetRegisterYank(GetSelectedText());
      }
      else
      {
        GetMacros().SetRegister(GetRegister(), GetSelectedText());
        GetSTC()->SelectNone();
      }
      break;
  }
  
  if (!m_InsertCommand.empty())
  {
    AppendInsertCommand(command.substr(0, parsed));
  }

  command = (m_Command.IsHandled() ? std::string(): command.substr(parsed));

  m_Count = 1; // restart with a new count

  return true;
}

bool wxExVi::OnChar(const wxKeyEvent& event)
{
  if (!GetIsActive())
  {
    return true;
  }
  else if (Mode().Insert())
  { 
    if (GetSTC()->SelectionIsRectangle() || 
        GetSTC()->GetSelectionMode() == wxSTC_SEL_THIN)
    {
      return true;
    }

    m_Command.Append(ConvertKeyEvent(event));
    const bool result = InsertMode(m_Command.Command());

    if (result || (GetSTC()->HexMode() && m_Command.size() > 2))
    {
      m_Command.clear();
    }

    return result && GetSTC()->GetOvertype();
  }
  else
  {
    if (!(event.GetModifiers() & wxMOD_ALT))
    {
      // This check is important, as WXK_NONE (0)
      // would add nullptr terminator at the end of m_Command,
      // and pressing ESC would not help, (rest is empty
      // because of the nullptr).
      if (event.GetUnicodeKey() != (wxChar)WXK_NONE)
      {
        if (!m_Command.empty() && 
             m_Command.front() == '@' && event.GetKeyCode() == WXK_BACK)
        {
          m_Command.pop_back();
        }
        else
        {
          if (m_Command.AppendExec(event.GetUnicodeKey()))
          {
            m_Command.clear();
          }
        }
      }
      else
      {
        return true;
      }
      
      return false;
    }
    else
    {
      return true;
    }
  }
}

bool wxExVi::OnKeyDown(const wxKeyEvent& event)
{
  if (!GetIsActive() || GetSTC()->AutoCompActive())
  {
    return true;
  }
  else if (!m_Command.empty() && m_Command.front() == '@')
  { 
    if (event.GetKeyCode() == WXK_BACK)
    {
      m_Command.pop_back();
      GetFrame()->StatusText(m_Command.Command().substr(1), "PaneMacro");
    }
    else if (event.GetKeyCode() == WXK_ESCAPE)
    {
      m_Command.clear();
      m_InsertCommand.clear();
      GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
    }

    return true;
  }
  else if (!event.HasAnyModifiers() &&
     (event.GetKeyCode() == WXK_BACK || event.GetKeyCode() == WXK_ESCAPE ||
     (!Mode().Visual() && event.GetKeyCode() == WXK_TAB) ||
     event.GetKeyCode() == WXK_RETURN ||
     event.GetKeyCode() == WXK_NUMPAD_ENTER ||
     (!Mode().Insert() && (
        event.GetKeyCode() == WXK_LEFT ||
        event.GetKeyCode() == WXK_DELETE ||
        event.GetKeyCode() == WXK_DOWN ||
        event.GetKeyCode() == WXK_UP ||
        event.GetKeyCode() == WXK_RIGHT ||
        event.GetKeyCode() == WXK_PAGEUP ||
        event.GetKeyCode() == WXK_PAGEDOWN))))
  {
    if (m_Command.AppendExec(ConvertKeyEvent(event)))
    {
      m_Command.clear();
      m_InsertCommand.clear();
      return false;
    }
    
    return true;
  }
  else if ((event.GetModifiers() & wxMOD_CONTROL) && event.GetKeyCode() != WXK_NONE)
  {
    if (const auto& it = GetMacros().GetKeysMap(KEY_CONTROL).find(event.GetKeyCode());
      it != GetMacros().GetKeysMap(KEY_CONTROL).end()) 
    {
      Command(it->second);
      return false;
    }

    return true;
  }
  else if ((event.GetModifiers() & wxMOD_ALT) && event.GetKeyCode() != WXK_NONE)
  {
    if (!Mode().Normal())
    {
      Command("\x1b");
    }

    if (const auto& it = GetMacros().GetKeysMap(KEY_ALT).find(event.GetKeyCode());
      it != GetMacros().GetKeysMap(KEY_ALT).end()) 
    {
      Command(it->second);
      return false;
    }

    return true;
  }
  else
  {
    return true;
  }
}

bool wxExVi::OtherCommand(std::string& command)
{
  if (command.empty())
  {
    return false;
  }

  if (const auto& it = std::find_if(
    m_OtherCommands.begin(), m_OtherCommands.end(), 
    [command](auto const& e) 
    {
      if (!isalpha(e.first.front())) 
      {
        for (const auto& r : e.first) 
        {
          if (r == command.front()) 
          {
            return true; 
          }
        }

        return false;
      }
      else
      {
        return e.first == command.substr(0, e.first.size());
      }
    });
    it != m_OtherCommands.end())
  { 
    if (const auto parsed = it->second(command); parsed > 0)
    {
      AppendInsertCommand(command.substr(0, parsed));
      command = command.substr(parsed);
      return true;
    }
  }

  return false;
}

bool wxExVi::ParseCommand(std::string& command)
{
  if (const auto& it = GetMacros().GetKeysMap().find(command.front());
    it != GetMacros().GetKeysMap().end()) 
  {
    command = it->second;
  }

  m_Count = 1;
  
  if (command.front() == '"')
  {
    if (command.size() < 2) return false;
    SetRegister(command[1]);
    command = command.substr(2);
  }
  else if (command.front() == ':')
  {
    return wxExEx::Command(command);
  }
  else
  {
    SetRegister(0);
    FilterCount(command);
  }

  MotionType motion = MOTION_NAVIGATE;
  bool check_other = true;

  switch (command.size())
  {
    case 0: return false;

    case 1: 
      if (m_Mode.Transition(command))
      {
        check_other = false;
      }
      else
      {
        m_InsertCommand.clear();
      }
      break;

    default: 
      if (OtherCommand(command))
      {
        return true;
      }

      switch (command[0])
      {
        case 'c': 
          motion = MOTION_CHANGE; 
          m_Mode.Transition(command);
          break;

        case 'd': 
          motion = MOTION_DELETE; 
          command = command.substr(1);
          break;

        case 'y': 
          motion = MOTION_YANK; 
          command = command.substr(1);
          break;

        default:
          if (m_Mode.Transition(command))
          {
            check_other = false;
          }
          else
          {
            m_InsertCommand.clear();
          }
      }
  }

  if (check_other && 
      !MotionCommand(motion, command) &&
      !OtherCommand(command))
  {
    return false;
  }

  if (!command.empty())
  {
    Mode().Insert() ? InsertMode(command): ParseCommand(command);
  }
  
  return true;
}

bool wxExVi::Put(bool after)
{
  if (GetRegisterText().empty())
  {
    return false;
  }

  // do not trim
  const bool yanked_lines = (wxExGetNumberOfLines(GetRegisterText(), false) > 1);

  if (yanked_lines)
  {
    if (after) 
    {
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0 || 
          GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->LineDown();
      }
    }
  
    GetSTC()->Home();
  }
  
  AddText(GetRegisterText());

  if (yanked_lines && after)
  {
    GetSTC()->LineUp();
  }

  return true;
}        

void wxExVi::SetLastCommand(const std::string& command)
{
  size_t first = 0;

  if (std::vector<std::string> v; 
    wxExMatch("^([1-9][0-9]*)(.*)", command, v) == 2)
  {
    first = v[0].size(); // skip a possible leading count
  }

  if (const auto& it = std::find(
    m_LastCommands.begin(), 
    m_LastCommands.end(), 
    command.substr(first, 1));
    it != m_LastCommands.end())
  {
    m_LastCommand = command;
  }
}
 
void wxExVi::VisualExtend(int begin_pos, int end_pos)
{
  if (begin_pos == wxSTC_INVALID_POSITION || end_pos == wxSTC_INVALID_POSITION)
  {
    return;
  }

  switch (Mode().Get())
  {
    case wxExViModes::VISUAL:
      GetSTC()->SetSelection(begin_pos, end_pos);
    break;

    case wxExViModes::VISUAL_LINE:
      if (begin_pos < end_pos)
      {
        GetSTC()->SetSelection(
          GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(begin_pos)), 
          GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(end_pos) + 1));
      }
      else
      {
        GetSTC()->SetSelection(
          GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(end_pos)), 
          GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(begin_pos) + 1));
      } 
    break;

    case wxExViModes::VISUAL_RECT:
      if (begin_pos < end_pos)
      {
        while (GetSTC()->GetCurrentPos() < end_pos)
        {
          GetSTC()->CharRightRectExtend();
        }
      }
      else
      {
        while (GetSTC()->GetCurrentPos() > begin_pos)
        {
          GetSTC()->CharLeftRectExtend();
        }
      }
    break;
  }
}
