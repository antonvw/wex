////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <regex>
#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/tokenzr.h>
#include <wx/extension/vi.h>
#include <wx/extension/addressrange.h>
#include <wx/extension/frd.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

// compares two strings in compile time constant fashion
constexpr int c_strcmp( char const* lhs, char const* rhs )
{
  return (('\0' == lhs[0]) && ('\0' == rhs[0])) ? 0
    :  (lhs[0] != rhs[0]) ? (lhs[0] - rhs[0])
    : c_strcmp( lhs+1, rhs+1 );
};

#if wxUSE_GUI

#define FOLD( )                                                          \
  const auto level = GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine()); \
  const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?        \
    GetSTC()->GetCurrentLine(): GetSTC()->GetFoldParent(GetSTC()->GetCurrentLine());\
  if (GetSTC()->GetFoldExpanded(line_to_fold) && command == "zc")        \
    GetSTC()->ToggleFold(line_to_fold);                                  \
  else if (!GetSTC()->GetFoldExpanded(line_to_fold) && command == "zo")  \
    GetSTC()->ToggleFold(line_to_fold);                                  \
  return true;
        
#define INDENT( )                                                      \
  switch (GetMode())                                                   \
  {                                                                    \
    case MODE_NORMAL:                                                  \
      wxExAddressRange(this, m_Count).Indent(command[0] == '>'); break;\
    case MODE_VISUAL:                                                  \
    case MODE_VISUAL_LINE:                                             \
    case MODE_VISUAL_RECT:                                             \
      wxExAddressRange(this, "'<,'>").Indent(command[0] == '>'); break;\
  }                                                                    \
  return true;                                                         \

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
      case WXK_NUMPAD_ENTER: c = WXK_RETURN; break;
      default: c = event.GetKeyCode();
    }
  }

  return c;
}

bool DeleteRange(wxExVi* vi, auto start, auto end)
{
  if (!vi->GetSTC()->GetReadOnly() && !vi->GetSTC()->HexMode())
  {
    const auto first = (start < end ? start: end);
    const auto last = (start < end ? end: start);
    const wxCharBuffer b(vi->GetSTC()->GetTextRangeRaw(first, last));
  
    vi->GetMacros().SetRegister(
      vi->GetRegister() ? vi->GetRegister(): '0', 
      std::string(b.data(), b.length()));
    
    vi->GetSTC()->DeleteRange(first, last - first);
  }
  
  return true;
}

// Returns true if after chr only one letter is followed.
bool OneLetterAfter(const std::string& chr, const std::string& letter)
{
  if (letter.size() < 1) return false;
  std::regex re("^" + chr + "[a-zA-Z]$");
  return std::regex_match(letter, re);
}

bool RegAfter(const wxString& text, const wxString& letter)
{
  std::regex re("^" + text.ToStdString() + "[0-9=\"a-z%.]$");
  return std::regex_match(letter.ToStdString(), re);
}

enum
{
  MOTION_CHANGE,
  MOTION_DELETE,
  MOTION_NAVIGATE,
  MOTION_YANK,
};

std::string wxExVi::m_LastFindCharCommand;

wxExVi::wxExVi(wxExSTC* stc)
  : wxExEx(stc)
  , m_Dot(false)
  , m_FSM(this, 
     // insert mode process
     [=](const std::string& command) {
        if (!m_Dot)
        {
          m_InsertText.clear();
          SetLastCommand((m_Count > 1 ? std::to_string(m_Count): "") + command, true);
        }
        GetSTC()->BeginUndoAction();},
     // back to normal mode process
     [=](const std::string& command) {
        if (!m_Dot)
        {
          const std::string lc(GetLastCommand() + GetRegisterInsert());
          SetLastCommand(lc + wxString(wxUniChar(WXK_ESCAPE)).ToStdString());
          // Record it (if recording is on).
          GetMacros().Record(lc);
          GetMacros().Record(wxString(wxUniChar(WXK_ESCAPE)).ToStdString());
        }
        m_Command.clear();
        GetSTC()->EndUndoAction();})
  , m_Count(1)
  , m_Start(0)
  , m_Type(MOTION_YANK)
  , m_SearchForward(true)
  , m_MotionCommands {
    {'b', [&](const std::string& command){MOTION(Word, Left, false, false);}},
    {'e', [&](const std::string& command){MOTION(Word, RightEnd, false, false);}},
    {'f', [&](const std::string& command){return FindChar(command);}},
    {'h', [&](const std::string& command){
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0) 
        MOTION(Char, Left, false, false); 
      return true;}},
    {'j', [&](const std::string& command){MOTION(Line, Down, false, false);}},
    {'k', [&](const std::string& command){MOTION(Line, Up, false, false);}},
    {'l', [&](const std::string& command){
      if (GetSTC()->GetCurrentPos() < 
          GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine())) 
        MOTION(Char, Right, false, false);
      return true;}},
    {'n', [&](const std::string& command){REPEAT(
      if (!GetSTC()->FindNext(
        wxExFindReplaceData::Get()->GetFindString(), GetSearchFlags(), 
        m_SearchForward))
      {
        m_Command.clear();
        return false;
      });
      return true;}},
    {'t', [&](const std::string& command){return FindChar(command);}}, 
    {'w', [&](const std::string& command){MOTION(Word, Right, false, false);}},
    {'B', [&](const std::string& command){MOTION(Word, Left, false, false);}},
    {'E', [&](const std::string& command){MOTION(Word, RightEnd, false, false);}},
    {'F', [&](const std::string& command){return FindChar(command);}},
    {'G', [&](const std::string& command){
      (m_Count == 1 ? 
         GetSTC()->DocumentEnd(): 
         GetSTC()->GotoLineAndSelect(m_Count));
       return true;}},
    {'H', [&](const std::string& command){
       GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine());
       return true;}},
    {'L', [&](const std::string& command){
       GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() - 1);
       return true;}},
    {'M', [&](const std::string& command){
       GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() / 2);
       return true;}},
    {'N', [&](const std::string& command){REPEAT(
      if (!GetSTC()->FindNext(
        wxExFindReplaceData::Get()->GetFindString(), GetSearchFlags(), 
        !m_SearchForward))
      {
        m_Command.clear();
        return false;
      });
      return true;}},
    {'T', [&](const std::string& command) {return FindChar(command);}},
    {'W', [&](const std::string& command){MOTION(Word, Right,false, false);}},
    {'/', [&](const std::string& command){return Find(command);}}, 
    {'?', [&](const std::string& command){return Find(command);}}, 
    {'\'', [&](const std::string& command){
      if (OneLetterAfter("'", command)) 
      {
        MarkerGoto(command.back()); 
        return true;
      } 
      else 
      {
        if (command.size() >= 2) return true;
        else return false;
      }}},
    {',', [&](const std::string& command){return FindChar(command);}}, 
    {' ', [&](const std::string& command){MOTION(Char, Right,false, false);}},
    {'0', [&](const std::string& command){MOTION(Line, Home, false, false);}},
    {'[', [&](const std::string& command){REPEAT(
        if (!GetSTC()->FindNext("{", GetSearchFlags(), false))
        {
          m_Command.clear();
          return false;
        })
        return true;}},
    {']', [&](const std::string& command){REPEAT(
        if (!GetSTC()->FindNext("{", GetSearchFlags(), true))
        {
          m_Command.clear();
          return false;
        })
        return true;}},
    {'(', [&](const std::string& command){MOTION(Para, Up,   false, false);}},
    {')', [&](const std::string& command){MOTION(Para, Down, false, false);}},
    {'{', [&](const std::string& command){MOTION(Para, Up,   false, false);}},
    {'}', [&](const std::string& command){MOTION(Para, Down, false, false);}},
    {'^', [&](const std::string& command){MOTION(Line, Home, false, false);}},
    {'+', [&](const std::string& command){MOTION(Line, Down, true,  true); }},
    {'|', [&](const std::string& command){
      GetSTC()->GotoPos(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()) + m_Count - 1);
      return true;}},
    {'-', [&](const std::string& command){MOTION(Line, Up,  true, true);}},
    {'$', [&](const std::string& command){MOTION(Line, End, false, false);}},
    {'%', [&](const std::string& command){
      auto pos = GetSTC()->GetCurrentPos();
      auto brace_match = GetSTC()->BraceMatch(pos);
      if (brace_match == wxSTC_INVALID_POSITION)
      {
        brace_match = GetSTC()->BraceMatch(--pos);
      }
      if (brace_match != wxSTC_INVALID_POSITION)
      {
        GetSTC()->GotoPos(brace_match);
        switch (GetMode())
        {
          case MODE_VISUAL:
            if (brace_match < pos)
              GetSTC()->SetSelection(brace_match, pos + 1);
            else
              GetSTC()->SetSelection(pos, brace_match + 1);
          break;
          case MODE_VISUAL_LINE:
            if (brace_match < pos)
            {
              GetSTC()->SetSelection(
                GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(brace_match)), 
                GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(pos) + 1));
            }
            else
            {
              GetSTC()->SetSelection(
                GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(pos)), 
                GetSTC()->PositionFromLine(GetSTC()->LineFromPosition(brace_match) + 1));
            } 
          break;
          case MODE_VISUAL_RECT:
            if (brace_match < pos)
            {
              while (GetSTC()->GetCurrentPos() < pos)
              {
                GetSTC()->CharRightRectExtend();
              }
            }
            else
            {
              while (GetSTC()->GetCurrentPos() > brace_match)
              {
                GetSTC()->CharLeftRectExtend();
              }
            }
          break;
        }
      }
      return true;}},
    {'_', [&](const std::string& command){MOTION(Line, Down, false, false);}},
    {';', [&](const std::string& command){return FindChar(command);}},
    {WXK_CONTROL_B, [&](const std::string& command){MOTION(Page, Up,         false, false);}},
    {WXK_CONTROL_F, [&](const std::string& command){MOTION(Page, Down,       false, false);}},
    {WXK_CONTROL_P, [&](const std::string& command){MOTION(Line, ScrollUp,   false, false);}},
    {WXK_CONTROL_Q, [&](const std::string& command){MOTION(Line, ScrollDown, false, false);}},
    {WXK_RETURN,    [&](const std::string& command){MOTION(Line, Down,       false, false);}}}
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
      if (wxString(command).Matches("r?"))
      {
        if (!GetSTC()->GetReadOnly())
        {
          if (GetSTC()->HexMode())
          {
            wxExHexModeLine ml(&GetSTC()->GetHexMode());
          
            if (!ml.Replace(command.back()))
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
      if (GetSTC()->CanUndo())
      {
        GetSTC()->Undo();
      }
      else
      {
        wxBell();
      }
      return true;}},
    {"x", [&](const std::string& command){
      return DeleteRange(this, GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + m_Count);}},
    {"D", [&](const std::string& command){
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
        GetSTC()->LineEndExtend();
        Cut();
      }
      return true;}},
    {"J", [&](const std::string& command){wxExAddressRange(this, m_Count).Join(); return true;}},
    {"P", [&](const std::string& command){Put(false); return true;}},
    {"X", [&](const std::string& command){
      return DeleteRange(this, GetSTC()->GetCurrentPos() - m_Count, GetSTC()->GetCurrentPos());}},
    {"Y", [&](const std::string& command){wxExAddressRange(this, m_Count).Yank(); return true;}},
    {"dd", [&](const std::string& command){
      (void)wxExAddressRange(this, m_Count).Delete(); return true;}},
    {"dgg", [&](const std::string& command){
      return DeleteRange(this, 0,
        GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()));}},
    {"gg", [&](const std::string& command){
      GetSTC()->DocumentStart(); return true;}},
    {"yy", [&](const std::string& command){
      wxExAddressRange(this, m_Count).Yank(); return true;}},
    {"zc", [&](const std::string& command){FOLD();}},
    {"zo", [&](const std::string& command){FOLD();}},
    {"zE", [&](const std::string& command){
      GetSTC()->SetLexerProperty("fold", "0");
      GetSTC()->Fold(false); return true;}},
    {"zf", [&](const std::string& command){
      GetSTC()->SetLexerProperty("fold", "1");
      GetSTC()->Fold(true); return true;}},
    {"ZZ", [&](const std::string& command){
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCloseEvent(wxEVT_CLOSE_WINDOW)); return true;}},
    {".", [&](const std::string& command){
      {
      m_Dot = true;
      const bool result = Command(GetLastCommand());
      m_Dot = false;
      return result;
      }}},
    {"~", [&](const std::string& command){REPEAT_WITH_UNDO(
      if (GetSTC()->GetReadOnly() || GetSTC()->HexMode()) return false;
      wxString text(GetSTC()->GetTextRange(
        GetSTC()->GetCurrentPos(), 
        GetSTC()->GetCurrentPos() + 1));
      wxIslower(text[0]) ? text.UpperCase(): text.LowerCase();
      GetSTC()->wxStyledTextCtrl::Replace(
        GetSTC()->GetCurrentPos(), 
        GetSTC()->GetCurrentPos() + 1, 
        text);
      GetSTC()->CharRight());
      return true;}},
    {"&", [&](const std::string& command){(void)Command(":.&");return true;}},
    {"*", [&](const std::string& command){FindWord();return true;}},
    {"#", [&](const std::string& command){FindWord(false);return true;}},
    {">", [&](const std::string& command){INDENT();}},
    {"<", [&](const std::string& command){INDENT();}},
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
        const wxString macro = command.back();
        if (GetMacros().IsRecorded(macro))
        {
          MacroPlayback(macro, m_Count);
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
        std::vector <wxString> v;
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
          wxString s;
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
    {"\x05", [&](const std::string& command){REPEAT_WITH_UNDO(ChangeNumber(true));}},
    {"\x07", [&](const std::string& command){
      GetFrame()->ShowExMessage(wxString::Format("%s line %d of %d --%d%%-- level %d", 
        GetSTC()->GetFileName().GetFullName().c_str(), 
        GetSTC()->GetCurrentLine() + 1,
        GetSTC()->GetLineCount(),
        100 * (GetSTC()->GetCurrentLine() + 1)/ GetSTC()->GetLineCount(),
        (GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK)
         - wxSTC_FOLDLEVELBASE));
      return true;}},
    {"\x08", [&](const std::string& command){
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode()) GetSTC()->DeleteBack();
      return true;}},
    {"\x0a", [&](const std::string& command){REPEAT_WITH_UNDO(ChangeNumber(false));}},
    {"\x12", [&](const std::string& command){
      if (command.size() == 1) return false;
      if (RegAfter(wxUniChar(WXK_CONTROL_R), command))
      {
        CommandReg(command[1]);
        return true;
      }  
      return false;}},
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

bool wxExVi::ChangeNumber(bool inc)
{
  if (GetSTC()->HexMode())
  {
    return false;
  }

  try 
  {
    const auto start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
    const auto sign = (GetSTC()->GetCharAt(start) == '-' ? 1: 0);
    const auto end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos() + sign, true);
    const std::string word(GetSTC()->GetTextRange(start, end).ToStdString());
    auto number = std::stoi(word, nullptr, 0);
    const auto next = (inc ? ++number: --number);
    
    std::ostringstream format;
    format.fill(' ');
    format.width(end - start);
    if (word.substr(0, 2) == "0x") format << std::hex;
    else if (word[0] == '0') format << std::oct;
    format << std::showbase << next;
  
    GetSTC()->wxStyledTextCtrl::Replace(start, end, format.str());
    
    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool wxExVi::Command(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }
  
  if ( command.front() == '=' ||
      (command.size() > 2 && wxString(command).StartsWith(wxUniChar(WXK_CONTROL_R) + wxString("="))))
  {
    CommandCalc(command);
    return true;
  }
  else if (ModeInsert())
  {
    InsertMode(command);
    return true;
  }
  
  bool handled = true;
  const auto size = GetSTC()->GetLength();
 
  if (!m_Dot && command.back() == WXK_ESCAPE)
  {
    if (!m_FSM.Transition("\x1b"))
    {
      wxBell();
    }
  }
  else
  {
    m_Count = 1;
    std::string rest(command);
    
    if (rest.front() == '"')
    {
      if (rest.size() < 2) return false;
      SetRegister(rest[1]);
      rest = rest.substr(2);
    }
    else
    {
      SetRegister(0);
      FilterCount(rest);
    }

    switch (rest.size())
    {
      case 0: return false;
      case 1: handled = CommandChar(rest); break;
      default: 
        switch (rest[0])
        {
          case 'c': handled = MotionCommand(MOTION_CHANGE, rest); break;
          case 'd': handled = MotionCommand(MOTION_DELETE, rest); break;
          case 'y': handled = MotionCommand(MOTION_YANK, rest); break;
          default: handled = MotionCommand(MOTION_NAVIGATE, rest); 
        }
        if (!handled)
        {
          handled = OtherCommand(rest);

          if (!handled)
          {
            handled = CommandChar(rest);
          }
        }
    }
  
    if (handled && ModeInsert())
    {
      if (!rest.empty())
      {
        InsertMode(rest);
      }
      
      return true;
    }
  }

  if (!handled)
  {  
    if (ModeVisual())
    {
      if (command.find("'<,'>") == std::string::npos)
      {
        return wxExEx::Command(command + "'<,'>");
      }
      else
      {
        return wxExEx::Command(command);
      }
    }
    else
    {
      return wxExEx::Command(command);
    }
  }
  
  if (!m_Dot)
  {
    // Set last command.
    SetLastCommand(command, 
      // Always when in insert mode,
      // or this was a file change command (so size different from before).
      ModeInsert() || size != GetSTC()->GetLength());
  }
    
  if (GetMacros().IsRecording() && command[0] != 'q' && command != "/")
  {
    GetMacros().Record(command);
  }  
  
  return true;
}

void wxExVi::CommandCalc(const wxString& command)
{
  const auto index = command.StartsWith("=") ? 1: 2;
  int width = 0;
  const auto sum = wxExCalculator(command.Mid(index).ToStdString(), this, width);

  if (ModeInsert())
  {
    if (wxString(GetLastCommand()).Matches("*c*"))
    {
      GetSTC()->ReplaceSelection(wxEmptyString);
    }
  
    AddText(wxString::Format("%.*f", width, sum).ToStdString());
  }
  else
  {
    GetFrame()->ShowExMessage(wxString::Format("%.*f", width, sum));
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
    case '=': GetFrame()->GetExCommand(this, reg); break;
    // clipboard register
    case '*': Put(true); break;
    // filename register
    case '%':
      if (ModeInsert())
      {
        AddText(GetSTC()->GetFileName().GetFullName().ToStdString());
      }
      else
      {
        wxExClipboardAdd(GetSTC()->GetFileName().GetFullPath());
      }
      break;
    default:
      if (!GetMacros().GetRegister(reg).empty())
      {
        AddText(GetMacros().GetRegister(reg));
        
        if (reg == '.')
        {
          m_InsertText += GetRegisterInsert();
        }
      }
      else
      {
        GetFrame()->ShowExMessage("?" + wxString(reg));
      }
  }
}

void wxExVi::FilterCount(std::string& command, const std::string& prefix)
{
  std::vector<wxString> v;
        
  const auto matches = wxExMatch("^" + prefix + "([1-9][0-9]*)(.*)", command, v);
  
  if (matches < 2)
  {
    return;
  }
  
  const auto count = std::stoi(v[matches - 2].ToStdString());
  
  if (count > 0)
  {
    m_Count *= count;
    command = (matches == 2 ? v[1]: v[0] + v[2]);
  }
}

bool wxExVi::Find(const std::string& command)
{
  m_SearchForward = command.front() == '/';
      
  if (command.length() > 1)
  {
    // This is a previous entered command.
    if (!GetSTC()->FindNext(
      command.substr(1),
      GetSearchFlags(),
      m_SearchForward))
    {
      return false;
    }
        
    wxExFindReplaceData::Get()->SetFindString(command.substr(1));
  }
  else
  {
    GetFrame()->GetExCommand(this, command + (ModeVisual() ? "'<,'>": ""));
  }
    
  return true;
}

bool wxExVi::FindChar(const std::string& text)
{
  if (text.empty()) 
  {
    return false;
  }
  
  char c; // char to find
  
  if ((text[0] == ';' || text[0] == ',') && text.size() == 1)
  {
    if (m_LastFindCharCommand.empty())
    {
      return false;
    }
    
    c = m_LastFindCharCommand.back();
  }
  else if (text.size() == 1)
  {
    return false;
  }
  else
  {
    c = text[1];
  }
  
  char d; // char specifying direction
  
  switch (text[0])
  {
    case ';': d = m_LastFindCharCommand.front(); break;
    case ',': 
      d = m_LastFindCharCommand.front();
      if (islower(d)) d = toupper(d);
      else d = tolower(d);
      break;
    default: 
      if (text.size() > 1) d = text.front();
      else d = m_LastFindCharCommand.front();
  }
  
  REPEAT(
    if (!GetSTC()->FindNext(c, 
      GetSearchFlags() & ~wxSTC_FIND_REGEXP, wxIslower(d)))
    {
      m_Command.clear();
      return false;
    });

  if (text[0] != ',' && text[0] != ';')
  {
    m_LastFindCharCommand = text;
  }

  if (tolower(d) == 't') GetSTC()->CharLeft();
  
  return true;
}
                
void wxExVi::FindWord(bool find_next)
{
  const auto start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
  const auto end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos(), true);
  
  wxExFindReplaceData::Get()->SetFindString(GetSTC()->GetTextRange(start, end));  
    
  GetSTC()->FindNext(
    "\\<"+ wxExFindReplaceData::Get()->GetFindString() + "\\>", 
    GetSearchFlags(), 
    find_next);
}

bool wxExVi::InsertMode(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }

  if (command.find(wxString(wxUniChar(WXK_CONTROL_R) + wxString("="))) !=
    std::string::npos)
  {
    if (
      command.compare(0, 2, 
        wxString(wxUniChar(WXK_CONTROL_R) + wxString("="))) == 0)
    {
      CommandCalc(command);
      return true;
    }
    else
    {
      InsertMode(wxString(command).BeforeFirst(wxUniChar(WXK_CONTROL_R)).ToStdString());
      CommandCalc(wxString(command).AfterFirst(wxUniChar(WXK_CONTROL_R)).ToStdString());
      return true;
    }
  }
  else if (command.find(wxString(wxUniChar(WXK_CONTROL_R)).ToStdString()) !=
    std::string::npos)
  {
    wxStringTokenizer tkz(command, wxUniChar(WXK_CONTROL_R));
    
    while (tkz.HasMoreTokens())
    {
      const wxString token = tkz.GetNextToken();
      
      if (RegAfter(wxUniChar(WXK_CONTROL_R), 
        wxUniChar(WXK_CONTROL_R) + tkz.GetString().Mid(0, 1)))
      {
        InsertMode(token.ToStdString());
        CommandReg(tkz.GetString().GetChar(0));
        InsertMode(tkz.GetString().Mid(1).ToStdString());
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
      
    case WXK_DELETE: 
      DeleteRange(this, GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + 1);
      break;
      
    case WXK_CONTROL_R:
      m_InsertText += command;
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

    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
      GetSTC()->NewLine();
        
      if (!GetSTC()->AutoCompActive())
      {
        m_InsertText += GetSTC()->GetEOL().ToStdString();
      }
      break;
      
    default: 
      if (wxString(GetLastCommand()).Matches("*c*") && m_InsertText.empty())
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
  
  return true;
}

void wxExVi::InsertModeNormal(const std::string& text)
{
  wxStringTokenizer tkz(text, "\r\n", wxTOKEN_RET_EMPTY);
  
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
  
      GetSTC()->AddText(tkz.GetLastDelimiter());
      GetSTC()->AutoIndentation(tkz.GetLastDelimiter());
    }
  }
  else
  {
    AddText(text);
  }
}

void wxExVi::MacroRecord(const std::string& text)
{
  if (ModeInsert())
  {
    m_InsertText += text;
  }
  else
  {
    wxExEx::MacroRecord(text);
  }
}

bool wxExVi::MotionCommand(int type, std::string& command)
{
  FilterCount(command, "([cdy])");
  
  const char c = (type == MOTION_NAVIGATE ? command[0]: command[1]);
  auto it = std::find_if(m_MotionCommands.begin(), m_MotionCommands.end(), 
    [c](auto const& e) {return e.first == c;});
  
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

      if (!it->second(command.substr(1))) return false;
    
      DeleteRange(this, m_Start, GetSTC()->GetCurrentPos());

      m_FSM.Transition(command);
      command = command.substr(2);
      break;
    
    case MOTION_DELETE:
      if (GetSTC()->GetReadOnly() || GetSTC()->HexMode())
      {
        return true;
      }
    
      if (!it->second(command.substr(1))) return false;
      
      DeleteRange(this, m_Start, GetSTC()->GetCurrentPos());
      break;
    
    case MOTION_NAVIGATE: 
      if (!it->second(command)) return false; 
      break;
    
    case MOTION_YANK:
      if (!ModeVisual())
      {
        m_FSM.Transition("v");
      }
    
      if (!it->second(command.substr(1))) return false;
    
      const auto end = GetSTC()->GetCurrentPos();
      GetSTC()->CopyRange(m_Start, end - m_Start);
      GetSTC()->SetSelection(m_Start, end);

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
    
    const bool result = InsertMode(std::string(1, ConvertKeyEvent(event)));
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
  else if (
    !event.HasAnyModifiers() &&
    (event.GetKeyCode() == WXK_BACK ||
     event.GetKeyCode() == WXK_ESCAPE ||
     event.GetKeyCode() == WXK_RETURN ||
     event.GetKeyCode() == WXK_NUMPAD_ENTER ||
     (!ModeVisual() && 
        event.GetKeyCode() == WXK_TAB) ||
     (!ModeInsert() &&
       (event.GetKeyCode() == WXK_LEFT ||
        event.GetKeyCode() == WXK_DELETE ||
        event.GetKeyCode() == WXK_DOWN ||
        event.GetKeyCode() == WXK_UP ||
        event.GetKeyCode() == WXK_RIGHT ||
        event.GetKeyCode() == WXK_PAGEUP ||
        event.GetKeyCode() == WXK_PAGEDOWN))))
  {
    if (!m_Command.empty() && m_Command.front() == '@')
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
      
      return false;
    }
    else
    {
      m_Command += ConvertKeyEvent(event);
    }
      
    const bool result = Command(m_Command);
    
    if (result)
    {
      m_Command.clear();
    }
    
    return !result;
  }
  else
  {
    return true;
  }
}

bool wxExVi::OtherCommand(std::string& command)
{
  auto it = std::find_if(m_OtherCommands.begin(), m_OtherCommands.end(), 
    [command](auto const& e) {return e.first == command.substr(0, e.first.size());});
  
  if (it == m_OtherCommands.end())
  {
    return false;
  }
  
  if (it->second(command))
  {
    command = command.substr(0, it->first.size());
    return true;
  }
  
  return false;
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
