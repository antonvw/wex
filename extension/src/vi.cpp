////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <sstream>
#include <string>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/extension/vi.h>
#include <wx/extension/address.h>
#include <wx/extension/frd.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

#define CHR_TO_NUM(c1,c2) ((c1 << 8) + c2)

#define NAVIGATE(REPEAT, SCOPE, DIRECTION, COND1, COND2, WRAP)           \
{                                                                        \
  if (m_Mode == MODE_VISUAL_LINE && GetSTC()->GetSelectedText().empty()) \
  {                                                                      \
    if ((#DIRECTION) == "Left" || (#DIRECTION) == "Up")                  \
    {                                                                    \
      GetSTC()->LineEnd();                                               \
      GetSTC()->HomeExtend();                                            \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      GetSTC()->Home();                                                  \
      GetSTC()->LineEndExtend();                                         \
    }                                                                    \
  }                                                                      \
  for (int i = 0; i < REPEAT; i++)                                       \
  {                                                                      \
    if (COND1)                                                           \
    {                                                                    \
      switch (m_Mode)                                                    \
      {                                                                  \
        case MODE_NORMAL:                                                \
          if (WRAP && (#SCOPE) == "Line")                                \
          {                                                              \
            if ((#DIRECTION) == "Down")                                  \
              GetSTC()->LineEnd();                                       \
            else                                                         \
              GetSTC()->Home();                                          \
          }                                                              \
          GetSTC()->SCOPE##DIRECTION();                                  \
          break;                                                         \
        case MODE_VISUAL: GetSTC()->SCOPE##DIRECTION##Extend();          \
          break;                                                         \
        case MODE_VISUAL_LINE:                                           \
          if ((#SCOPE) != "Char" && (#SCOPE) != "Word")                  \
            GetSTC()->SCOPE##DIRECTION##Extend();                        \
          break;                                                         \
        case MODE_VISUAL_RECT: GetSTC()->SCOPE##DIRECTION##RectExtend(); \
          break;                                                         \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  if ((#SCOPE) == "Line")                                                \
  {                                                                      \
    switch (m_Mode)                                                      \
    {                                                                    \
      case MODE_NORMAL:                                                  \
        if ((COND2) &&                                                   \
          GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) !=              \
          GetSTC()->GetLineIndentation(GetSTC()->GetCurrentLine()))      \
          GetSTC()->VCHome(); break;                                     \
      case MODE_VISUAL:                                                  \
        if (COND2) GetSTC()->VCHomeExtend(); break;                      \
      case MODE_VISUAL_LINE:                                             \
        if ((#DIRECTION) == "Up")                                        \
          GetSTC()->HomeExtend();                                        \
        else                                                             \
          GetSTC()->LineEndExtend();                                     \
        break;                                                           \
    }                                                                    \
  }                                                                      \
};                                                                       \

char ConvertKeyEvent(const wxKeyEvent& event)
{
  char c;
  
  switch (event.GetKeyCode())
  {
    case WXK_LEFT:     c = 'h'; break;
    case WXK_DOWN:     c = 'j'; break;
    case WXK_UP:       c = 'k'; break;
    case WXK_RIGHT:    c = 'l'; break;
    case WXK_PAGEUP:   c = WXK_CONTROL_B; break;
    case WXK_PAGEDOWN: c = WXK_CONTROL_F; break;
    case WXK_NUMPAD_ENTER: c = WXK_RETURN; break;
    default: c = event.GetKeyCode();
  }
  
  return c;
}

void DeleteRange(wxExVi* vi, int start, int end)
{
  if (!vi->GetSTC()->GetReadOnly() && !vi->GetSTC()->HexMode())
  {
    const wxCharBuffer b(vi->GetSTC()->GetTextRangeRaw(start, end));
  
    vi->GetMacros().SetRegister(
      vi->GetRegister() ? vi->GetRegister(): '0', 
      std::string(b.data(), b.length()));
    
    vi->GetSTC()->DeleteRange(start, end - start);
  }
}

void DeleteRange(wxExVi* vi, int repeat, std::function<void()> process)
{
  const int start = vi->GetSTC()->GetCurrentPos();
  
  for (int i = 0; i < repeat; i++) 
    process();
                  
  DeleteRange(vi, start, vi->GetSTC()->GetCurrentPos());
}
                
void YankRange(wxExVi* vi, int repeat, std::function<void()> process)
{
  for (int i = 0; i < repeat; i++) 
    process();

  if (!vi->GetRegister())
  {
    vi->SetRegisterYank(vi->GetSelectedText());
  }
  else
  {
    vi->GetMacros().SetRegister(vi->GetRegister(), vi->GetSelectedText());
    vi->GetSTC()->SelectNone();
  }
}
              
bool YankedLines(wxExVi* vi)
{
  // do not trim
  return wxExGetNumberOfLines(vi->GetRegisterText(), false) > 1;
}

// Returns true if after text only one letter is followed.
bool OneLetterAfter(const wxString text, const wxString& letter)
{
  return wxRegEx("^" + text + "[a-zA-Z]$").Matches(letter);
}

bool RegAfter(const wxString text, const wxString& letter)
{
  return wxRegEx("^" + text + "[0-9=\"a-z%.]$").Matches(letter);
}

std::string wxExVi::m_LastFindCharCommand;

wxExVi::wxExVi(wxExSTC* stc)
  : wxExEx(stc)
  , m_Dot(false)
  , m_Mode(MODE_NORMAL)
  , m_InsertRepeatCount(1)
  , m_SearchForward(true)
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
  
  const int start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
  const int end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos(), true);
  const wxString word = GetSTC()->GetTextRange(start, end);
  
  long number;
  
  if (word.ToLong(&number))
  {
    const long next = (inc ? ++number: --number);
    
    if (next >= 0)
    {
      std::ostringstream format;
      format.fill('0');
      format.width(end - start);
      format << next;
    
      GetSTC()->wxStyledTextCtrl::Replace(start, end, format.str());
    }
    else
    {
      GetSTC()->wxStyledTextCtrl::Replace(start, end, 
        wxString::Format("%d", next));
    }
    
    return true;
  }
  
  return false;
}

bool wxExVi::Command(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }
  
  if (
      command.front() == '=' ||
     (GetMacros().IsPlayback() &&
      wxString(command).StartsWith(wxUniChar(WXK_CONTROL_R) + wxString("="))))
  {
    CommandCalc(command);
    return true;
  }
  else if (m_Mode == MODE_INSERT)
  {
    InsertMode(command);
    return true;
  }
  
  bool handled = true;

  const int size = GetSTC()->GetLength();
 
  switch ((int)command[0])
  {
    // Cannot be at CommandChar, as 0 is stripped from rest.
    case '0':
    case '^':
      switch (m_Mode)
      {
        case MODE_NORMAL: GetSTC()->Home(); break;
        case MODE_VISUAL: GetSTC()->HomeExtend(); break;
        case MODE_VISUAL_RECT: GetSTC()->HomeRectExtend(); break;
      }
      break;
    case 'G': GetSTC()->DocumentEnd(); break;
      
    case '/':
    case '?':
      m_SearchForward = command.front() == '/';
          
      if (command.length() > 1)
      {
        // This is a previous entered command.
        handled = GetSTC()->FindNext(
          command.substr(1),
          GetSearchFlags(),
          m_SearchForward);
            
        if (handled)
        {
          GetMacros().Record(command);
        }
      }
      else
      {
        if (m_Mode >= MODE_VISUAL)
        {
          GetFrame()->GetExCommand(this, command + "'<,'>");
        }
        else
        {
          GetFrame()->GetExCommand(this, command);
        }
      }
      return handled;

    default: 
      // Handle ESCAPE: deselects and clears command buffer.
      if (!m_Dot && command.back() == WXK_ESCAPE)
      {
        bool action = false;
        
        if (!GetSTC()->GetSelectedText().empty() || 
             GetSTC()->SelectionIsRectangle())
        {
          GetSTC()->SelectNone();
          action = true;
        }
        
        if (m_Mode == MODE_NORMAL)
        {
          m_Command.clear();
        }
        else
        {
          m_Mode = MODE_NORMAL;
          action = true;
        }
      
        if (!action)
        {
          wxBell();
        }
      }
      // Handle multichar commands.
      else
      {
        std::string rest(command);
        long int repeat = 1;
        
        if (rest.front() == '"')
        {
          if (rest.size() < 2)
          {
            return false;
          }
          
          SetRegister(rest[1]);
          rest = rest.substr(2);
        }
        else
        {
          SetRegister(0);

          int seq_size = 0; // size of sequence of digits from begin in rest
          
          for (size_t i = 0; i < rest.size(); i++)
          {
            if (rest[i] > 255 || rest[i] < 0 || !isdigit(rest[i]))
              break;
            seq_size++;
          }
          
          if (seq_size > 0)
          {
            repeat = strtol(rest.substr(0, seq_size).c_str(), NULL, 10);
            rest = rest.substr(seq_size);
          }
        }
  
        switch (rest.size())
        {
          case 0: return false;
          case 1: 
            handled = CommandChar((int)rest[0], repeat); 
            if (handled)
            {
              rest = rest.substr(1);
            }
            break;
          default: handled = CommandChars(rest, repeat);
        }
      
        if (handled && m_Mode == MODE_INSERT)
        {
          if (!rest.empty())
          {
            InsertMode(rest);
          }
          
          return true;
        }
      } // Handle multichar commands.
  } // switch (command[0])

  if (!handled)
  {  
    if (m_Mode >= MODE_VISUAL)
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
      m_Mode == MODE_INSERT || size != GetSTC()->GetLength());

    // Record it (if recording is on).
    GetMacros().Record(command);
  }
    
  return true;
}

void wxExVi::CommandCalc(const wxString& command)
{
  const int index = command.StartsWith("=") ? 1: 2;
  
  // Calculation register.
  int width = 0;
  const double sum = wxExCalculator(command.Mid(index), this, width);

  if (m_Mode == MODE_INSERT)
  {
    if (wxString(GetLastCommand()).EndsWith("cw"))
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

bool wxExVi::CommandChar(int c, int repeat)
{
  switch (c)
  {
    // Insert commands.
    case 'a': 
    case 'i': 
    case 'o': 
    case 'A': 
    case 'C': 
    case 'I': 
    case 'O': 
    case 'R': 
      SetInsertMode((char)c, repeat); 
      break;

    // Navigate char and line commands.
    case 'h': 
    case WXK_LEFT:
              NAVIGATE(repeat, Char, Left,     GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0, false, false); break;
    case 'l': 
    case ' ': 
    case WXK_RIGHT:
              NAVIGATE(repeat, Char, Right,    GetSTC()->GetCurrentPos() < GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine()), false, false); break;
    case 'j': 
    case WXK_DOWN:
              NAVIGATE(repeat, Line, Down,     GetSTC()->GetCurrentLine() < GetSTC()->GetNumberOfLines(), false, false); break;
    case 'k': 
    case WXK_UP:
              NAVIGATE(repeat, Line, Up,       GetSTC()->GetCurrentLine() > 0, false, false); break;
              
    // Navigate line commands that pass wrapped lines.
    case '+': 
    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
              NAVIGATE(repeat, Line, Down,     true, true, true); break;
    case '-': 
              NAVIGATE(repeat, Line, Up,       GetSTC()->GetCurrentLine() > 0, true, true); break;
              
    // Navigate word commands (B, W, E, treated as b, w, e for the moment).
    case 'B':
    case 'b': NAVIGATE(repeat, Word, Left,     true, false, false); break;
    case 'W':
    case 'w': NAVIGATE(repeat, Word, Right,    true, false, false); break;
    case 'E':
    case 'e': NAVIGATE(repeat, Word, RightEnd, true, false, false); break;
    
    // Navigate paragraph commands ((,) treated as as {,}).
    case '(':
    case '{': NAVIGATE(repeat, Para, Up,       true, false, false); break;
    case ')':
    case '}': NAVIGATE(repeat, Para, Down,     true, false, false); break;
    case WXK_CONTROL_B:
    
    // Navigate page commands.
    case WXK_PAGEUP:
              NAVIGATE(repeat, Page, Up,       true, false, false); break;
    case WXK_CONTROL_F:
    case WXK_PAGEDOWN:
              NAVIGATE(repeat, Page, Down,     true, false, false); break;
        
    case 'n': 
      for (int i = 0; i < repeat; i++) 
        if (!GetSTC()->FindNext(
          wxExFindReplaceData::Get()->GetFindString(), 
          GetSearchFlags(), 
          m_SearchForward)) break;
      break;

    case 'p': Put(true); break;
      
    case 'q': 
      if (GetMacros().IsRecording())
      {
        GetMacros().StopRecording();
      }
      else
      {
        return false;
      }
      break;
      
    case 'u': 
      if (GetSTC()->CanUndo())
      {
        GetSTC()->Undo();
      }
      else
      {
        wxBell();
      }
      break;
    
    case 'v': m_Mode = MODE_VISUAL; break;
      
    case 'x': 
      DeleteRange(this, GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos() + repeat);
      break;
        
    case 'D': 
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
        GetSTC()->LineEndExtend();
        Cut();
      }
      break;
        
    case 'G': GetSTC()->GotoLineAndSelect(repeat); break;
    case 'H': GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine()); break;
        
    case 'J':
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
        GetSTC()->BeginUndoAction();
        GetSTC()->SetTargetStart(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()));
        GetSTC()->SetTargetEnd(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine() + repeat));
        GetSTC()->LinesJoin();
        GetSTC()->EndUndoAction();
      }
      break;
        
    case 'L': GetSTC()->GotoLine(
      GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() - 1); 
      break;
        
    case 'M': GetSTC()->GotoLine(
      GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() / 2);
      break;
        
    case 'N': 
      for (int i = 0; i < repeat; i++) 
        if (!GetSTC()->FindNext(
          wxExFindReplaceData::Get()->GetFindString(), 
          GetSearchFlags(), 
          !m_SearchForward)) break;
      break;
        
    case 'P': Put(false); break;
    case 'V': m_Mode = MODE_VISUAL_LINE; break;
      
    case 'X': 
      DeleteRange(this, GetSTC()->GetCurrentPos() - repeat, GetSTC()->GetCurrentPos());
      break;

    case 'Z': m_Mode = MODE_VISUAL_RECT; break;
    
    case '.': 
      {
      m_Dot = true;
      const bool result = Command(GetLastCommand());
      m_Dot = false;
      return result;
      }
      break;
        
    case ';': 
      {
      m_Dot = true;
      const bool result = Command(m_LastFindCharCommand); 
      m_Dot = false;
      return result;
      }
      break;
        
    case '~': return ToggleCase();
    case '%': GotoBrace(); break;
    case '*': FindWord(); break;
    case '#': FindWord(false); break;

    case '|': 
      GetSTC()->GotoPos(
        GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()) + repeat - 1);
      break;
    
    case '$': 
      switch (m_Mode)
      {
        case MODE_NORMAL: GetSTC()->LineEnd(); break;
        case MODE_VISUAL: GetSTC()->LineEndExtend(); break;
        case MODE_VISUAL_RECT: GetSTC()->LineEndRectExtend(); break;
      }
      break;
    
    case WXK_CONTROL_E: 
      for (int i = 0; i < repeat; i++) ChangeNumber(true); 
      break;
    case WXK_CONTROL_G:
      GetFrame()->ShowExMessage(wxString::Format("%s line %d of %d --%d%%-- level %d", 
        GetSTC()->GetFileName().GetFullName().c_str(), 
        GetSTC()->GetCurrentLine() + 1,
        GetSTC()->GetLineCount(),
        100 * (GetSTC()->GetCurrentLine() + 1)/ GetSTC()->GetLineCount(),
        (GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK)
         - wxSTC_FOLDLEVELBASE));
      break;
    case WXK_CONTROL_J: 
      for (int i = 0; i < repeat; i++) ChangeNumber(false); 
      break;
    case WXK_CONTROL_P: // (^y is not possible, already redo accel key)
      for (int i = 0; i < repeat; i++) GetSTC()->LineScrollUp(); 
      break;
    case WXK_CONTROL_Q: // (^n is not possible, already new doc accel key)
      for (int i = 0; i < repeat; i++) GetSTC()->LineScrollDown(); 
      break;
        
    case WXK_BACK:
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode()) GetSTC()->DeleteBack();
      break;
      
    case WXK_TAB:
      // just ignore tab, except on first col, then it indents
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) == 0)
      {
        m_Command.clear();
        return false;
      }
      break;
      
    case '[': 
    case ']': 
      for (int i = 0; i < repeat; i++) 
        if (!GetSTC()->FindNext("{", GetSearchFlags(), c == ']'))
        {
          m_Command.clear();
          return false;
        }
      break;
      
    default:
      return false;
  }
  
  return true;
}

bool wxExVi::CommandChars(std::string& command, int repeat)
{
  switch (CHR_TO_NUM((int)command[0], (int)command[1]))
  {
    case CHR_TO_NUM('c','c'):
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
         GetSTC()->Home();
         GetSTC()->DelLineRight();
  
         if (!SetInsertMode("cc", repeat))
         {
           return false;
         }
         command = command.substr(2);
      }
      break;
    case CHR_TO_NUM('c','w'):
      // do not use CanCopy 
      if (!GetSTC()->HexMode() && !GetSTC()->GetReadOnly())
      {
        if (!GetSTC()->GetSelectedText().empty())
        {
          GetSTC()->SetCurrentPos(GetSTC()->GetSelectionStart());
        }
 
        for (int i = 0; i < repeat; i++) GetSTC()->WordRightEndExtend();
 
        if (!SetInsertMode("cw", repeat))
        {
          return false;
        }
        command = command.substr(2);
      }
      break;
          
    case CHR_TO_NUM('d','d'): wxExAddressRange(this, repeat).Delete(); break;
    case CHR_TO_NUM('d','e'): DeleteRange(this, repeat, [&](){GetSTC()->WordRightEnd();}); break;
    case CHR_TO_NUM('d','h'): DeleteRange(this, repeat, [&](){GetSTC()->CharLeft();}); break;
    case CHR_TO_NUM('d','j'): DeleteRange(this, repeat, [&](){GetSTC()->LineDown();}); break;
    case CHR_TO_NUM('d','k'): DeleteRange(this, repeat, [&](){GetSTC()->LineUp();}); break;
    case CHR_TO_NUM('d','l'): DeleteRange(this, repeat, [&](){GetSTC()->CharRight();}); break;
    case CHR_TO_NUM('d','w'): DeleteRange(this, repeat, [&](){GetSTC()->WordRight();}); break;
    case CHR_TO_NUM('d','G'): DeleteRange(this, GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()), GetSTC()->GetLastPosition()); break;
    case CHR_TO_NUM('d','0'): DeleteRange(this, GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()), GetSTC()->GetCurrentPos()); break;
    case CHR_TO_NUM('d','$'): DeleteRange(this, GetSTC()->GetCurrentPos(), GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine())); break;
     
    case CHR_TO_NUM('g','g'): GetSTC()->DocumentStart(); break;
    
    case CHR_TO_NUM('y','e'): YankRange(this, repeat, [&](){GetSTC()->WordRightEndExtend();}); break;
    case CHR_TO_NUM('y','h'): YankRange(this, repeat, [&](){GetSTC()->CharLeftExtend();}); break;
    case CHR_TO_NUM('y','j'): YankRange(this, repeat, [&](){GetSTC()->LineDownExtend();}); break;
    case CHR_TO_NUM('y','k'): YankRange(this, repeat, [&](){GetSTC()->LineUpExtend();}); break;
    case CHR_TO_NUM('y','l'): YankRange(this, repeat, [&](){GetSTC()->CharRightExtend();}); break;
    case CHR_TO_NUM('y','w'): YankRange(this, repeat, [&](){GetSTC()->WordRightExtend();}); break;
    case CHR_TO_NUM('y','y'): wxExAddressRange(this, repeat).Yank(); break;
    case CHR_TO_NUM('y','0'): YankRange(this, repeat, [&](){GetSTC()->HomeExtend();}); break;
    case CHR_TO_NUM('y','$'): YankRange(this, repeat, [&](){GetSTC()->LineEndExtend();}); break;
    
    case CHR_TO_NUM('z','c'):
    case CHR_TO_NUM('z','o'):
    {
      const int level = GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine());
      const int line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
        GetSTC()->GetCurrentLine(): GetSTC()->GetFoldParent(GetSTC()->GetCurrentLine());
      if (GetSTC()->GetFoldExpanded(line_to_fold) && command == "zc")
        GetSTC()->ToggleFold(line_to_fold);
      else if (!GetSTC()->GetFoldExpanded(line_to_fold) && command == "zo")
        GetSTC()->ToggleFold(line_to_fold);
    }
    break;
    case CHR_TO_NUM('z','E'):
    case CHR_TO_NUM('z','f'):
      GetSTC()->SetLexerProperty("fold", (int)command[1] == 'f' ? "1": "0");
      GetSTC()->Fold(command[1] == 'f');
      break;
    case CHR_TO_NUM('Z','Z'):
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
      wxPostEvent(wxTheApp->GetTopWindow(), 
        wxCloseEvent(wxEVT_CLOSE_WINDOW));
      break;
    case CHR_TO_NUM('>','>'):
    case CHR_TO_NUM('<','<'):
      switch (m_Mode)
      {
        case MODE_NORMAL: wxExAddressRange(this, repeat).Indent(command == ">>"); break;
        case MODE_VISUAL: 
        case MODE_VISUAL_LINE: 
        case MODE_VISUAL_RECT: 
          wxExAddressRange(this, "'<,'>").Indent(command == ">>"); break;
      }
      break;
    case CHR_TO_NUM('@','@'): MacroPlayback(GetMacros().GetMacro(), repeat); break;
         
    default:
      if (FindChar(repeat, command, "f"))
      {
        m_LastFindCharCommand = command;
      }
      else if (FindChar(repeat, command, "t"))
      {
        GetSTC()->CharLeft();
        m_LastFindCharCommand = command;
      }
      else if (command.front() == 'm')
      {
        if (OneLetterAfter("m", command))
        {
          MarkerAdd(command.back());
        }
        else
        {
          m_Command.clear();
          return false;
        }
      }
      else if (OneLetterAfter("q", command))
      {
        if (!GetMacros().IsRecording())
        {
          MacroStartRecording(command.substr(1));
          return true; // as we should not do default actions
        }
      } 
      else if (wxString(command).Matches("r?"))
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
            GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + repeat);
            GetSTC()->ReplaceTarget(wxString(command.back(), repeat));
          }
        }
      }
      else if (OneLetterAfter("'", command))
      {
        MarkerGoto(command.back());
      }
      else if (RegAfter("@", command))
      {
        const wxString macro = command.back();
        
        if (GetMacros().IsRecorded(macro))
        {
          MacroPlayback(macro, repeat);
        }
        else
        {
          m_Command.clear();
          GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
          return false;
        }
      }
      else if (RegAfter(wxUniChar(WXK_CONTROL_R), command))
      {
        CommandReg(command[1]);
        return true;
      }  
      else if (CommandChar((int)command[0], repeat))
      {
        command = command.substr(1);
      }
      else if (command.front() == '@')
      {
        std::vector <wxString> v;
          
        if (wxExMatch("@([a-zA-Z].+)@", command, v) > 0)
        {
          if (!MacroPlayback(v[0], repeat))
          {
            m_Command.clear();
            GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
            return false;
          }
        }
        else if (GetMacros().StartsWith(command.substr(1)))
        {
          wxString s;
          
          if (wxExAutoComplete(command.substr(1), GetMacros().Get(), s))
          {
            GetFrame()->StatusText(s, "PaneMacro");
            
            if (!MacroPlayback(s, repeat))
            {
              m_Command.clear();
              GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
            }
          }
          else
          {
            GetFrame()->StatusText(command.substr(1), "PaneMacro");
            return false;
          }
        }
        else
        {
          m_Command.clear();
          GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
        }
      }
      else if (command == "dgg")
      {
        DeleteRange(this, 
          0,
          GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine())); 
      }
      else
      {
        return false;
      }
  }
  
  return true;
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
      if (m_Mode == MODE_INSERT)
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

bool wxExVi::FindChar(int repeat, const wxString& text, const wxString& start)
{
  if (text.StartsWith(start) || text.StartsWith(start.Upper()))
  {
    for (int i = 0; i < repeat; i++) 
    {
      const int flags = GetSearchFlags() & ~wxSTC_FIND_REGEXP;
      
      if (!GetSTC()->FindNext(text.Last(), flags, wxIslower(text[0])))
      {
        m_Command.clear();
        return false;
      }
    }
    
    return true;
  }
  
  return false;
}
                
void wxExVi::FindWord(bool find_next)
{
  const int start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
  const int end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos(), true);
  
  wxExFindReplaceData::Get()->SetFindString(GetSTC()->GetTextRange(start, end));  
    
  GetSTC()->FindNext(
    "\\<"+ wxExFindReplaceData::Get()->GetFindString() + "\\>", 
    GetSearchFlags(), 
    find_next);
}

void wxExVi::GotoBrace()
{
  int pos = GetSTC()->GetCurrentPos();
  int brace_match = GetSTC()->BraceMatch(pos);
          
  if (brace_match == wxSTC_INVALID_POSITION)
  {
    brace_match = GetSTC()->BraceMatch(--pos);
  }

  if (brace_match != wxSTC_INVALID_POSITION)
  {
    GetSTC()->GotoPos(brace_match);
  
    switch (m_Mode)
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
      
    case WXK_CONTROL_R:
      m_InsertText += command;
      break;
        
    case WXK_ESCAPE:
      // Add extra inserts if necessary.        
      if (!m_InsertText.empty())
      {
        for (int i = 1; i < m_InsertRepeatCount; i++)
        {
          AddText(m_InsertText);
        }
      
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
            for (int i = 1; i <= m_InsertRepeatCount; i++)
            {
              AddText(rest);
            }
          }
          else
          {
            wxString text;
            
            GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
            
            for (int i = 1; i <= m_InsertRepeatCount; i++)
            {
              text += rest;
            }
            
            GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + text.size());
            GetSTC()->ReplaceTarget(text);
          }
        }
      }
        
      GetSTC()->EndUndoAction();
      
      if (!m_Dot)
      {
        const std::string lc(GetLastCommand() + GetRegisterInsert());
        
        SetLastCommand(lc + wxString(wxUniChar(WXK_ESCAPE)).ToStdString());
          
        // Record it (if recording is on).
        GetMacros().Record(lc);
        GetMacros().Record(wxString(wxUniChar(WXK_ESCAPE)).ToStdString());
      }
      
      m_Mode = MODE_NORMAL;
      
      GetSTC()->SetOvertype(false);
      
      if (!GetSTC()->GetSelectedText().empty() ||
           GetSTC()->SelectionIsRectangle())
      {
        GetSTC()->SelectNone();
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
      if (wxString(GetLastCommand()).EndsWith("cw") && m_InsertText.empty())
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
          wxStringTokenizer tkz(command, "\r\n", wxTOKEN_RET_EMPTY);
          
          if (command.find('\0') == std::string::npos && tkz.HasMoreTokens())
          {
            while (tkz.HasMoreTokens())
            {
              const wxString token(tkz.GetNextToken());
              
              if (!token.empty())
              {
                GetSTC()->AddText(token);
              }
          
              GetSTC()->AddText(tkz.GetLastDelimiter());
              GetSTC()->AutoIndentation(tkz.GetLastDelimiter());
            }
          }
          else
          {
            AddText(command);
          }
        }
        
        if (!m_Dot)
        {
          m_InsertText += command;
        }
      }
  }
  
  return true;
}

void wxExVi::MacroRecord(const std::string& text)
{
  if (m_Mode == MODE_INSERT)
  {
    m_InsertText += text;
  }
  else
  {
    wxExEx::MacroRecord(text);
  }
}

bool wxExVi::OnChar(const wxKeyEvent& event)
{
  if (!GetIsActive())
  {
    return true;
  }
  else if (m_Mode == MODE_INSERT)
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
      // would add NULL terminator at the end of m_Command,
      // and pressing ESC would not help, (rest is empty
      // because of the NULL).
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
     event.GetKeyCode() == WXK_TAB ||
     (m_Mode != MODE_INSERT &&
       (event.GetKeyCode() == WXK_LEFT ||
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

bool wxExVi::Put(bool after)
{
  if (GetRegisterText().empty())
  {
    return false;
  }

  if (YankedLines(this))
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

  if (YankedLines(this) && after)
  {
    GetSTC()->LineUp();
  }

  return true;
}        

bool wxExVi::SetInsertMode(
  const wxString& c, 
  int repeat)
{
  if (GetSTC()->GetReadOnly() || GetSTC()->HexMode())
  {
    return false;
  }
    
  m_Mode = MODE_INSERT;
  
  if (!m_Dot)
  {
    m_InsertText.clear();
    
    if (repeat > 1)
    {
      SetLastCommand(wxString::Format("%d%s", repeat, c.c_str()).ToStdString(), true);
    }
    else
    {
      SetLastCommand(c.ToStdString(), true);
    }
  }
  
  GetSTC()->BeginUndoAction();
  
  switch ((int)c.GetChar(0))
  {
    case 'a': 
      GetSTC()->CharRight(); 
      break;

    case 'c': 
    case 'i': 
      break;

    case 'o': 
      GetSTC()->LineEnd(); 
      GetSTC()->NewLine(); 
      break;
      
    case 'A': GetSTC()->LineEnd(); 
      break;

    case 'C': 
      GetSTC()->LineEndExtend();
      Cut();
      break;
      
    case 'I': 
      GetSTC()->Home(); 
      break;

    case 'O': 
      GetSTC()->Home(); 
      GetSTC()->NewLine(); 
      GetSTC()->LineUp(); 
      break;

    case 'R': 
      GetSTC()->SetOvertype(true);
      break;

    default: wxFAIL;
  }

  if (c.GetChar(0) == 'c')
  {
    m_InsertRepeatCount = 1;
  }
  else
  {
    m_InsertRepeatCount = repeat;
  }
  
  return true;
}

bool wxExVi::ToggleCase()
{
  // Toggle case in hex mode not yet supported.
  if (GetSTC()->GetReadOnly() || GetSTC()->HexMode())
  {
    return false;
  }
    
  wxString text(GetSTC()->GetTextRange(
    GetSTC()->GetCurrentPos(), 
    GetSTC()->GetCurrentPos() + 1));

  wxIslower(text[0]) ? text.UpperCase(): text.LowerCase();

  GetSTC()->wxStyledTextCtrl::Replace(
    GetSTC()->GetCurrentPos(), 
    GetSTC()->GetCurrentPos() + 1, 
    text);

  GetSTC()->CharRight();
  
  return true;
}

#endif // wxUSE_GUI
