////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <regex>
#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
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
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

// compares two strings in compile time constant fashion
constexpr int c_strcmp( char const* lhs, char const* rhs )
{
  return (('\0' == lhs[0]) && ('\0' == rhs[0])) ? 0
    :  (lhs[0] != rhs[0]) ? (lhs[0] - rhs[0])
    : c_strcmp( lhs+1, rhs+1 );
};

void SelExtend(wxExVi* vi, int old_pos, int new_pos)
{
  if (old_pos == wxSTC_INVALID_POSITION || new_pos == wxSTC_INVALID_POSITION)
  {
    return;
  }

  switch (vi->GetMode())
  {
    case wxExVi::MODE_VISUAL:
      if (new_pos < old_pos)
        vi->GetSTC()->SetSelection(new_pos, old_pos + 1);
      else
        vi->GetSTC()->SetSelection(old_pos, new_pos + 1);
    break;

    case wxExVi::MODE_VISUAL_LINE:
      if (new_pos < old_pos)
      {
        vi->GetSTC()->SetSelection(
          vi->GetSTC()->PositionFromLine(vi->GetSTC()->LineFromPosition(new_pos)), 
          vi->GetSTC()->PositionFromLine(vi->GetSTC()->LineFromPosition(old_pos) + 1));
      }
      else
      {
        vi->GetSTC()->SetSelection(
          vi->GetSTC()->PositionFromLine(vi->GetSTC()->LineFromPosition(old_pos)), 
          vi->GetSTC()->PositionFromLine(vi->GetSTC()->LineFromPosition(new_pos) + 1));
      } 
    break;

    case wxExVi::MODE_VISUAL_RECT:
      if (new_pos < old_pos)
      {
        while (vi->GetSTC()->GetCurrentPos() < old_pos)
        {
          vi->GetSTC()->CharRightRectExtend();
        }
      }
      else
      {
        while (vi->GetSTC()->GetCurrentPos() > new_pos)
        {
          vi->GetSTC()->CharLeftRectExtend();
        }
      }
    break;
  }
}

#define MOTION(SCOPE, DIRECTION, COND, WRAP)                           \
{                                                                      \
  for (auto i = 0; i < m_Count; i++)                                   \
  {                                                                    \
    switch (GetMode())                                                 \
    {                                                                  \
      case MODE_NORMAL:                                                \
        if (WRAP && c_strcmp((#SCOPE), "Line") ==0)                    \
        {                                                              \
          if (c_strcmp((#DIRECTION), "Down") == 0)                     \
            GetSTC()->LineEnd();                                       \
          else                                                         \
            GetSTC()->Home();                                          \
        }                                                              \
        GetSTC()->SCOPE##DIRECTION();                                  \
        break;                                                         \
      case MODE_VISUAL: GetSTC()->SCOPE##DIRECTION##Extend();          \
        break;                                                         \
      case MODE_VISUAL_LINE:                                           \
        if (c_strcmp((#SCOPE), "Char") != 0 &&                         \
            c_strcmp((#SCOPE), "Word") != 0)                           \
          GetSTC()->SCOPE##DIRECTION##Extend();                        \
        break;                                                         \
      case MODE_VISUAL_RECT: GetSTC()->SCOPE##DIRECTION##RectExtend(); \
        break;                                                         \
    }                                                                  \
  }                                                                    \
  if (c_strcmp((#SCOPE), "Line") == 0)                                 \
  {                                                                    \
    switch (GetMode())                                                 \
    {                                                                  \
      case MODE_NORMAL:                                                \
        if ((COND) &&                                                  \
          GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) !=            \
          GetSTC()->GetLineIndentation(GetSTC()->GetCurrentLine()))    \
          GetSTC()->VCHome(); break;                                   \
      case MODE_VISUAL:                                                \
        if (COND) GetSTC()->VCHomeExtend(); break;                     \
    }                                                                  \
  }                                                                    \
  return true;                                                         \
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
  return true;                       \
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

bool OneLetterAfter(const std::string& text, const std::string& letter)
{
  return std::regex_match(letter, std::regex("^" + text + "[a-zA-Z]$"));
}

bool RegAfter(const std::string& text, const std::string& letter)
{
  return std::regex_match(letter, std::regex("^" + text + "[0-9=\"a-z%._]$"));
}

enum
{
  MOTION_CHANGE,
  MOTION_DELETE,
  MOTION_NAVIGATE,
  MOTION_YANK,
};

#define ESC "\x1b"

std::string wxExVi::m_LastFindCharCommand;

wxExVi::wxExVi(wxExSTC* stc)
  : wxExEx(stc)
  , m_FSM(this, 
     // insert mode process
     [=](const std::string& command) {
        if (!m_Dot)
        {
          m_InsertText.clear();
          m_CommandKeep = (m_Count > 1 ? std::to_string(m_Count): "") + command;
        }
        GetSTC()->BeginUndoAction();},
     // back to normal mode process
     [=](const std::string& command) {
        if (!m_Dot)
        {
          const std::string lc(m_CommandKeep + GetRegisterInsert());
          SetLastCommand(lc + ESC);
          // Record it (if recording is on).
          GetMacros().Record(lc);
          GetMacros().Record(ESC);
        }
        m_Command.clear();
        GetSTC()->EndUndoAction();})
  , m_Type(MOTION_NAVIGATE)
  , m_MotionCommands {
    {"bB", [&](const std::string& command){MOTION(Word, Left, false, false);}},
    {"eE", [&](const std::string& command){MOTION(Word, RightEnd, false, false);}},
    {"fFtT,;", [&](const std::string& command){
      if (command.empty()) return false;
      char c; // char to find
      if ((command[0] == ';' || command[0] == ',') && command.size() == 1)
      {
        if (m_LastFindCharCommand.empty()) return false;
        c = m_LastFindCharCommand.back();
      }
      else if (command.size() == 1)
      {
        return false;
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
          GetSearchFlags() & ~wxSTC_FIND_REGEXP, 
          islower(d) > 0))
        {
          m_Command.clear();
          return false;
        });
      if (command[0] != ',' && command[0] != ';')
      {
        m_LastFindCharCommand = command;
      }
      if (tolower(d) == 't') GetSTC()->SelectNone();
      return true;}},
    {"h", [&](const std::string& command){
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0) 
        MOTION(Char, Left, false, false); 
      return true;}},
    {"j", [&](const std::string& command){MOTION(Line, Down, false, false);}},
    {"k", [&](const std::string& command){MOTION(Line, Up, false, false);}},
    {"l ", [&](const std::string& command){
      if (command == "l" && GetSTC()->GetCurrentPos() >= 
          GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine())) return true; 
      MOTION(Char, Right, false, false);}},
    {"nN", [&](const std::string& command){REPEAT(
      if (!GetSTC()->FindNext(
        wxExFindReplaceData::Get()->GetFindString(), GetSearchFlags(), 
        command == "n"? m_SearchForward: !m_SearchForward))
      {
        m_Command.clear();
        return false;
      });
      return true;}},
    {"wW", [&](const std::string& command){MOTION(Word, Right, false, false);}},
    {"G", [&](const std::string& command){
       (m_Count == 1 ? 
         GetSTC()->DocumentEnd(): 
         (void)wxExSTCData(GetSTC()).Control(wxExControlData().Line(m_Count)).Inject());
       return true;}},
    {"H", [&](const std::string& command){
       GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine());
       return true;}},
    {"L", [&](const std::string& command){
       GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() - 1);
       return true;}},
    {"M", [&](const std::string& command){
       GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() / 2);
       return true;}},
    {"/?", [&](const std::string& command){
      m_SearchForward = command.front() == '/';

      if (command.length() > 1)
      {
        if (command[1] == WXK_CONTROL_R)
        {
          if (command.length() < 3)
          {
            return false;
          }

          if (!GetSTC()->FindNext(
            GetMacros().GetRegister(command[2]),
            GetSearchFlags(),
            m_SearchForward)) return false;
          wxExFindReplaceData::Get()->SetFindString(GetMacros().GetRegister(command[2]));
          return true;
        }
        
        // This is a previous entered command.
        if (!GetSTC()->FindNext(
          command.substr(1),
          GetSearchFlags(),
          m_SearchForward)) return false;
        wxExFindReplaceData::Get()->SetFindString(command.substr(1));
        return true;
      }
      else
      {
        return GetFrame()->GetExCommand(this, command + (ModeVisual() ? "'<,'>": ""));
      }}},
    {"\'", [&](const std::string& command){
      if (OneLetterAfter("'", command)) 
      {
        const auto pos = GetSTC()->GetCurrentPos();
        MarkerGoto(command.back()); 
        SelExtend(this, pos, GetSTC()->GetCurrentPos());
        return true;
      } 
      return command.size() >= 2;}},
    {"0^", [&](const std::string& command){MOTION(Line, Home, false, false);}},
    {"[]", [&](const std::string& command){REPEAT(
        if (!GetSTC()->FindNext("{", GetSearchFlags(), false))
        {
          m_Command.clear();
          return false;
        })
        return true;}},
    {"({", [&](const std::string& command){MOTION(Para, Up,   false, false);}},
    {")}", [&](const std::string& command){MOTION(Para, Down, false, false);}},
    {"+", [&](const std::string& command){MOTION(Line, Down, true,  true); }},
    {"|", [&](const std::string& command){
      GetSTC()->GotoPos(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()) + m_Count - 1);
      return true;}},
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
        SelExtend(this, pos, brace_match);
      }
      return true;}},
    {"\r_", [&](const std::string& command){MOTION(Line, Down, false, false);}},
    {"\x02", [&](const std::string& command){MOTION(Page, Up,         false, false);}},
    {"\x06", [&](const std::string& command){MOTION(Page, Down,       false, false);}},
    {"\x10", [&](const std::string& command){MOTION(Line, ScrollUp,   false, false);}},
    {"\x11", [&](const std::string& command){MOTION(Line, ScrollDown, false, false);}}}
  , m_OtherCommands {
    {"m", [&](const std::string& command){
      if (OneLetterAfter("m", command))
      {
        MarkerAdd(command.back());
        return true;
      }
      return false;}},
    {"p", [&](const std::string& command){Put(true);return true;}},
    {"q", [&](const std::string& command){
      if (GetMacros().IsRecording())
      {
        GetMacros().StopRecording();
        return true;
      }
      else if (OneLetterAfter("q", command))
      {
        if (!GetMacros().IsRecording())
        {
          MacroStartRecording(command.substr(1));
        }
        return true;
      } 
      return false;}},
    {"r", [&](const std::string& command){
      if (command.size() > 1)
      {
        if (!GetSTC()->GetReadOnly())
        {
          if (GetSTC()->HexMode())
          {
            if (!GetSTC()->GetHexMode().Replace(command.back()))
            {
              m_Command.clear();
              return false;
            }
          }
          else
          {
            GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
            GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + m_Count);
            GetSTC()->ReplaceTarget(wxString(command.back(), m_Count));
          }
        }
        return true;
      }
      return false;}},
    {"u", [&](const std::string& command){
      if (GetSTC()->CanUndo()) GetSTC()->Undo();
      else wxBell();
      return true;}},
    {"x", [&](const std::string& command){
      return DeleteRange(this, GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + m_Count);}},
    {"J", [&](const std::string& command){wxExAddressRange(this, m_Count).Join(); return true;}},
    {"P", [&](const std::string& command){Put(false);return true;}},
    {"Q", [&](const std::string& command){
      GetFrame()->SaveCurrentPage("ctags");
      GetCTags()->Find(
          GetSTC()->GetSelectedText().empty() ? 
            GetSTC()->GetWordAtPos(GetSTC()->GetCurrentPos()):
            GetSTC()->GetSelectedText().ToStdString());
      return true;}},
    {"S", [&](const std::string& command){
      GetFrame()->RestorePage("ctags");
      return true;}},
    {"X", [&](const std::string& command){
      return DeleteRange(this, GetSTC()->GetCurrentPos() - m_Count, GetSTC()->GetCurrentPos());}},
    {"dd", [&](const std::string& command){
      if (wxExAddressRange(this, m_Count).Delete())
      {
        m_FSM.Transition("\x1b");
      }
      return true;}},
    {"dgg", [&](const std::string& command){
      return DeleteRange(this, 0, GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()));}},
    {"gg", [&](const std::string& command){
      GetSTC()->DocumentStart(); return true;}},
    {"yy", [&](const std::string& command){
      if (wxExAddressRange(this, m_Count).Yank())
      {
        m_FSM.Transition("\x1b");
      }
      return true;}},
    {"ZZ", [&](const std::string& command){
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCloseEvent(wxEVT_CLOSE_WINDOW)); return true;}},
    {"z", [&](const std::string& command){
      if (command.size() <= 1) return false;
      const auto level = GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine());
      const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
        GetSTC()->GetCurrentLine(): GetSTC()->GetFoldParent(GetSTC()->GetCurrentLine());
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
          for (int i = 0; i < GetSTC()->GetLineCount(); i++) GetSTC()->EnsureVisible(i);
          break;
      }; return true;}},
    {".", [&](const std::string& command){
      m_Dot = true;
      const bool result = Command(GetLastCommand());
      m_Dot = false;
      return result;}},
    {"~", [&](const std::string& command){
      if (GetSTC()->GetLength() == 0 || GetSTC()->GetReadOnly() || GetSTC()->HexMode()) return false;
      REPEAT_WITH_UNDO(
        if (GetSTC()->GetCurrentPos() == GetSTC()->GetLength()) return false;
        wxString text(GetSTC()->GetTextRange(
            GetSTC()->GetCurrentPos(), 
            GetSTC()->GetCurrentPos() + 1));
        if (text.empty()) return false;
        islower(text[0]) ? text.UpperCase(): text.LowerCase();
        GetSTC()->wxStyledTextCtrl::Replace(
          GetSTC()->GetCurrentPos(), 
          GetSTC()->GetCurrentPos() + 1, 
          text);
        GetSTC()->CharRight());
      return true;}},
    {"><", [&](const std::string& command){
      switch (GetMode())
      {
        case MODE_NORMAL:
          wxExAddressRange(this, m_Count).Indent(command == ">"); break;
        case MODE_VISUAL:
        case MODE_VISUAL_LINE:
        case MODE_VISUAL_RECT:
          wxExAddressRange(this, "'<,'>").Indent(command == ">"); break;
      }
      return true;}},
    {"*#!", [&](const std::string& command){
      const auto start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
      const auto end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos(), true);
      const std::string word(GetSTC()->GetSelectedText().empty() ?
        GetSTC()->GetTextRange(start, end).ToStdString():
        GetSTC()->GetSelectedText());
      if (command == "!")
      {
        wxExBrowserSearch(word);
      }
      else
      {
        wxExFindReplaceData::Get()->SetFindString(word);
        GetSTC()->FindNext(
          "\\<"+ wxExFindReplaceData::Get()->GetFindString() + "\\>", 
          GetSearchFlags(), 
          command == "*");
      }
      return true;}},
    {"\t", [&](const std::string& command){
      // just ignore tab, except on first col, then it indents
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) == 0)
      {
        m_Command.clear();
        return false;
      }
      return true;}},
    {"@", [&](const std::string& command){
      if (command.size() == 1) return false;
      if (command == "@@") 
      {
        MacroPlayback(GetMacros().GetMacro(), m_Count);
        return true;
      }
      else if (RegAfter("@", command))
      {
        const std::string macro = std::string(1, command.back());
        if (GetMacros().IsRecorded(macro))
        {
          if (!MacroPlayback(macro, m_Count))
          {
            m_Command.clear();
            return false;
          }
        }
        else
        {
          m_Command.clear();
          GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
          return false;
        }
        return true;
      }
      else
      {
        std::vector <std::string> v;
        if (wxExMatch("@([a-zA-Z].+)@", command, v) > 0)
        {
          if (!MacroPlayback(v[0], m_Count))
          {
            m_Command.clear();
            GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
          }
          else
          {
            return true;
          }
        }
        else if (GetMacros().StartsWith(command.substr(1)))
        {
          std::string s;
          if (wxExAutoComplete(command.substr(1), GetMacros().Get(), s))
          {
            GetFrame()->StatusText(s, "PaneMacro");
            
            if (!MacroPlayback(s, m_Count))
            {
              m_Command.clear();
              GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
            }
            else
            {
              return true;
            }
          }
          else
          {
            GetFrame()->StatusText(command.substr(1), "PaneMacro");
          }
        }
        else
        {
          m_Command.clear();
          GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
          return true;
        }
      }
      return false;}},
    // ctrl-e, ctrl-j
    {"\x05\x0a", [&](const std::string& command){REPEAT_WITH_UNDO(
        if (GetSTC()->HexMode()) return true;
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
        } return true;)}},
    // ctrl-h
    {"\x08", [&](const std::string& command){
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode()) GetSTC()->DeleteBack();
      return true;}},
    // ctrl-r
    {"\x12", [&](const std::string& command){
      if (command.size() > 1 &&
        RegAfter(std::string(1, WXK_CONTROL_R), command))
      {
        CommandReg(command[1]);
        return true;
      }  
      return false;}},
    // delete char
    {"\x7F", [&](const std::string& command){
      return DeleteRange(this, GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + m_Count);}}}
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

bool wxExVi::Command(const std::string& command, bool is_handled)
{
  if (command.empty() || !GetIsActive())
  {
    return false;
  }

  if (ModeVisual() && command.find("'<,'>") == std::string::npos &&
    wxExEx::Command(command + "'<,'>"))
  {
    return true;
  }
  else if (command.front() == '=' ||
-   (command.size() > 2 && 
-    command.find(std::string(1, WXK_CONTROL_R) + "=") == 0))
  {
    CommandCalc(command);
    GetMacros().Record(command);
    return true;
  }
  else if (ModeInsert())
  {
    return InsertMode(command);
  }
  else if (!m_Dot && command.back() == WXK_ESCAPE)
  {
    m_FSM.Transition("\x1b");
    return true;
  }

  const auto size = GetSTC()->GetLength();
  std::string parse(command);

  if (!ParseCommand(parse, is_handled))
  {
    return false;
  }

  if (!m_Dot)
  {
    // Set last command.
    SetLastCommand(command, 
      // Always when in insert mode,
      // or this was a file change command (so size different from before).
      ModeInsert() || size != GetSTC()->GetLength());
  }
    
  if (GetMacros().IsRecording() && command[0] != 'q' && command != "/" && command != "?")
  {
    GetMacros().Record(command);
  }  

  return true;
}

void wxExVi::CommandCalc(const std::string& command)
{
  const auto index = command[0] == '=' ? 1: 2;
  int width = 0;
  const auto sum = Calculator(command.substr(index), width);
  if (std::isnan(sum)) return;

  if (ModeInsert())
  {
    if (GetLastCommand().find('c') != std::string::npos)
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

bool wxExVi::CommandChar(std::string& command)
{
  if (
    m_FSM.Transition(command.substr(0, 1)) ||
    MotionCommand(MOTION_NAVIGATE, command) ||
    OtherCommand(command))
  {
    command = command.substr(1);
    return true;
  }
  
  return false;
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
      if (ModeInsert())
      {
        Put(true); 
      }
      break;
    // filename register
    case '%':
      if (ModeInsert())
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
        if (ModeInsert())
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

void wxExVi::FilterCount(std::string& command, const std::string& prefix)
{
  std::vector<std::string> v;
        
  const auto matches = wxExMatch("^" + prefix + "([1-9][0-9]*)(.*)", command, v);
  
  if (matches >= 2)
  {
    const auto count = std::stoi(v[matches - 2]);
    
    if (count > 0)
    {
      m_Count *= count;
      command = (matches == 2 ? v[1]: v[0] + v[2]);
    }
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
      if (m_FSM.Transition("\x1b"))
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
      InsertMode(wxExFirstOf(command, std::string(1, WXK_CONTROL_R), 0, FIRST_OF_BEFORE));
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

    wxExTokenizer tkz(command, std::string(1, WXK_CONTROL_R), false);
    
    while (tkz.HasMoreTokens())
    {
      const std::string token = tkz.GetNextToken();
      const std::string rest(tkz.GetString());
      
      if (RegAfter("\x12", "\x12" + rest.substr(0, 1)))
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
        m_InsertText.erase(m_InsertText.size() - 1);
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
        const std::string rest(command.substr(0, command.size() - 1));
        
        if (!GetSTC()->GetSelectedText().empty())
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
        
      if (m_FSM.Transition("\x1b"))
      {
        GetSTC()->SetOvertype(false);
      }
      break;

    default: 
      if (GetLastCommand().find('c') != std::string::npos && m_InsertText.empty())
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
          m_InsertText.erase(m_InsertText.size() - 1);
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
  wxExTokenizer tkz(text, "\r\n", false);
  
  if (text.find('\0') == std::string::npos && tkz.HasMoreTokens())
  {
    while (tkz.HasMoreTokens())
    {
      std::string token(tkz.GetNextToken());
      
      if (!token.empty())
      {
        if (token.back() == ' ' || token.back() == '\t' || token.back() == ';')
        {
          if (!m_InsertText.empty())
          {
            const size_t last(m_InsertText.find_last_of(" ;\t"));
            const auto& it = GetMacros().GetAbbreviations().find(m_InsertText.substr(last + 1));
            
            if (it != GetMacros().GetAbbreviations().end())
            {
              m_InsertText.replace(last + 1, it->first.length(), it->second);
              
              const auto pos = GetSTC()->GetCurrentPos();
              const auto match_pos = GetSTC()->FindText(
                pos,
                GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()),
                it->first);
                
              if (match_pos != wxSTC_INVALID_POSITION)
              {
                GetSTC()->SetTargetStart(match_pos);
                GetSTC()->SetTargetEnd(match_pos + it->first.length());
                GetSTC()->ReplaceTarget(it->second);
                GetSTC()->SetCurrentPos(pos + it->second.length() - it->first.length());
              }
            }
          }
          else
          {
            const size_t last(token.find_last_of(" ;\t", token.size() - 2));
            const std::string word(token.substr(last + 1, token.size() - 2 - last));
            const auto& it = GetMacros().GetAbbreviations().find(word);
            
            if (it != GetMacros().GetAbbreviations().end())
            {
              token.replace(last + 1, it->first.length(), it->second);
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

bool wxExVi::MotionCommand(int type, std::string& command, bool is_handled)
{
  FilterCount(command, "([cdy])");
  
  const char c = (type == MOTION_NAVIGATE ? command[0]: command[1]);
  const auto& it = std::find_if(m_MotionCommands.begin(), m_MotionCommands.end(), 
    [c](auto const& e) {for (const auto& r : e.first) if (r == c) return true; return false;});
  
  if (it == m_MotionCommands.end())
  {
    return false;
  }

  if ((command == "/" || command == "?") || 
      (command[0] != '/' && command[0] != '?'))
  {
    m_Start = GetSTC()->GetCurrentPos();
    m_Type = type;
  }
  
  switch (m_Type)
  {
    case MOTION_CHANGE:
      if (!GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->SetCurrentPos(GetSTC()->GetSelectionStart());
      }
    
      if (!ModeVisual())
      {
        m_FSM.Transition("v");
      }

      if (!is_handled && !it->second(command.substr(1))) return false;
    
      DeleteRange(this, m_Start, GetSTC()->GetCurrentPos());

      m_FSM.Transition(command);
      command = command.substr(2);
      break;
    
    case MOTION_DELETE:
      if (GetSTC()->GetReadOnly())
      {
        return true;
      }
    
      if (!is_handled && !it->second(command.substr(1))) return false;
      
      DeleteRange(this, m_Start, GetSTC()->GetCurrentPos());
      command = command.substr(2);
      break;
    
    case MOTION_NAVIGATE: 
      if (!is_handled && !it->second(command)) return false; 
      break;
    
    case MOTION_YANK:
      if (!ModeVisual())
      {
        m_FSM.Transition("v");
      }
    
      if (!is_handled && !it->second(command.substr(1))) return false;
    
      auto end = GetSTC()->GetCurrentPos();
    
      if (end - m_Start > 0)
      {
        GetSTC()->CopyRange(m_Start, end - m_Start);
        GetSTC()->SetSelection(m_Start, end);
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
      
        GetSTC()->CopyRange(end, m_Start - end);
        GetSTC()->SetSelection(end, m_Start);
      }

      m_FSM.Transition("\x1b");
      
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
  
  m_Count = 1;

  return true;
}

bool wxExVi::OnChar(const wxKeyEvent& event)
{
  if (!GetIsActive())
  {
    return true;
  }
  else if (ModeInsert())
  { 
    if (GetSTC()->SelectionIsRectangle() || 
        GetSTC()->GetSelectionMode() == wxSTC_SEL_THIN)
    {
      return true;
    }

    m_Command += std::string(1, ConvertKeyEvent(event));
    
    const bool result = InsertMode(m_Command);

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
          m_Command.resize(m_Command.size() - 1);
        }
        else
        {
          m_Command += event.GetUnicodeKey();
        }
      
        if (Command(m_Command))
        {
          m_Command.clear();
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
      m_Command.resize(m_Command.size() - 1);
      GetFrame()->StatusText(m_Command.substr(1), "PaneMacro");
    }
    else if (event.GetKeyCode() == WXK_ESCAPE)
    {
      m_Command.clear();
      GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
    }
    
    return true;
  }
  else if (!event.HasAnyModifiers() &&
     (event.GetKeyCode() == WXK_BACK || event.GetKeyCode() == WXK_ESCAPE ||
     (!ModeVisual() && event.GetKeyCode() == WXK_TAB) ||
     event.GetKeyCode() == WXK_RETURN ||
     event.GetKeyCode() == WXK_NUMPAD_ENTER ||
     (!ModeInsert() && (
        event.GetKeyCode() == WXK_LEFT ||
        event.GetKeyCode() == WXK_DELETE ||
        event.GetKeyCode() == WXK_DOWN ||
        event.GetKeyCode() == WXK_UP ||
        event.GetKeyCode() == WXK_RIGHT ||
        event.GetKeyCode() == WXK_PAGEUP ||
        event.GetKeyCode() == WXK_PAGEDOWN))))
  {
    m_Command += ConvertKeyEvent(event);
    const bool result = Command(m_Command);

    if (result)
    {
      m_Command.clear();
    }
    
    return !result;
  }
  else if ((event.GetModifiers() & wxMOD_CONTROL) && event.GetKeyCode() != WXK_NONE)
  {
    const auto& it = GetMacros().GetKeysMap(KEY_CONTROL).find(event.GetKeyCode());

    if (it != GetMacros().GetKeysMap(KEY_CONTROL).end()) 
    {
      Command(it->second);
      return false;
    }

    return true;
  }
  else if ((event.GetModifiers() & wxMOD_ALT) && event.GetKeyCode() != WXK_NONE)
  {
    if (!ModeNormal())
    {
      Command("\x1b");
    }

    const auto& it = GetMacros().GetKeysMap(KEY_ALT).find(event.GetKeyCode());

    if (it != GetMacros().GetKeysMap(KEY_ALT).end()) 
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

bool wxExVi::OtherCommand(std::string& command) const
{
  const auto& it = std::find_if(m_OtherCommands.begin(), m_OtherCommands.end(), 
    [command](auto const& e) {
    if (!isalpha(e.first.front())) {
      for (const auto& r : e.first) if (r == command.front()) return true; return false;}
    else
      return e.first == command.substr(0, e.first.size());});
  
  if (it != m_OtherCommands.end() && it->second(command))
  {
    command = command.substr(0, it->first.size());
    return true;
  }

  return false;
}

bool wxExVi::ParseCommand(std::string& command, bool is_handled)
{
  const auto& it = GetMacros().GetKeysMap().find(command.front());

  if (it != GetMacros().GetKeysMap().end()) 
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

  int parsed_chars = 0;

  bool handled = true;

  switch (command.size())
  {
    case 0: return false;
    case 1: handled = CommandChar(command); break;
    default: 
      switch (command[0])
      {
        case 'c': handled = MotionCommand(MOTION_CHANGE, command, is_handled); 
          break;

        case 'd': handled = MotionCommand(MOTION_DELETE, command, is_handled); 
          break;

        case 'y': handled = MotionCommand(MOTION_YANK, command, is_handled); 
          parsed_chars = 2;
          break;

        default: handled = MotionCommand(MOTION_NAVIGATE, command, is_handled); 
          parsed_chars = (command[0] == '/' || command[0] == '?' ? 0: 1);
          if (command.front() == '\'')
          {
            parsed_chars = 2;
          }
          break;
      }

      if (!handled)
      {
        handled = OtherCommand(command);

        if (!handled)
        {
          handled = CommandChar(command);
        }
      }
  }

  if (handled && ModeInsert())
  {
    if (!command.empty())
    {
      InsertMode(command);
    }
    
    return true;
  }

  if (!handled)
  {  
    return false;
  }

  if (!command.empty() && parsed_chars > 0)
  {
    command = command.substr(parsed_chars);
    ParseCommand(command, is_handled);
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
#endif // wxUSE_GUI
