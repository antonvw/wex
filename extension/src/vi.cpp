////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

#define CHR_TO_NUM(c1,c2) ((c1 << 8) + c2)

#define NAVIGATE(REPEAT, SCOPE, DIRECTION, COND1, COND2)                 \
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
        case MODE_NORMAL: GetSTC()->SCOPE##DIRECTION();                  \
          break;                                                         \
        case MODE_VISUAL: GetSTC()->SCOPE##DIRECTION##Extend();          \
          break;                                                         \
        case MODE_VISUAL_LINE:                                           \
          if ((#SCOPE) != "Char" && (#SCOPE) != "Word")                  \
            GetSTC()->SCOPE##DIRECTION##Extend();                        \
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
    default: c = event.GetKeyCode();
  }
  
  return c;
}

// Returns true if after text only one letter is followed.
bool OneLetterAfter(const wxString text, const wxString& letter)
{
  return wxRegEx("^" + text + "[a-zA-Z]$").Matches(letter);
}

bool RegAfter(const wxString text, const wxString& letter)
{
  return wxRegEx("^" + text + "[0-9=\"a-z%]$").Matches(letter);
}

wxExSTCEntryDialog* wxExVi::m_Dialog = NULL;
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
        if (m_Mode == MODE_VISUAL || m_Mode == MODE_VISUAL_LINE)
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
        
        if (!GetSTC()->GetSelectedText().empty())
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
          
          for (int i = 0; i < rest.size(); i++)
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
          default: 
          switch (CHR_TO_NUM((int)rest[0], (int)rest[1]))
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
                rest = rest.substr(2);
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
                rest = rest.substr(2);
              }
              break;
            case CHR_TO_NUM('d','d'): wxExAddressRange(this, repeat).Delete(); break;
            case CHR_TO_NUM('d','e'):
              if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
              {
                const int start = GetSTC()->GetCurrentPos();
                for (int i = 0; i < repeat; i++) 
                  GetSTC()->WordRightEnd();
                  
                if (!GetRegister())
                {
                  GetSTC()->SetSelection(start, GetSTC()->GetCurrentPos());
                  GetSTC()->Cut();
                }
                else
                {
                  SetRegisterRange(start, GetSTC()->GetCurrentPos());
                  GetSTC()->DeleteRange(start, GetSTC()->GetCurrentPos() - start);
                }
              }
              break;
            case CHR_TO_NUM('d','w'):
              if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
              {
                const int start = GetSTC()->GetCurrentPos();
                for (int i = 0; i < repeat; i++) 
                  GetSTC()->WordRight();
                  
                if (!GetRegister())
                {
                  GetSTC()->SetSelection(start, GetSTC()->GetCurrentPos());
                  GetSTC()->Cut();
                }
                else
                {
                  SetRegisterRange(start, GetSTC()->GetCurrentPos());
                  GetSTC()->DeleteRange(start, GetSTC()->GetCurrentPos() - start);
                }
              }
              break;
            case CHR_TO_NUM('d','G'): Command(".,$d"); break;
            case CHR_TO_NUM('d','0'):
              if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
              {
                if (!GetRegister())
                {
                  GetSTC()->HomeExtend();
                  GetSTC()->Cut();
                }
                else
                {
                  const int start = GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine());
                  SetRegisterRange(start, GetSTC()->GetCurrentPos());
                  GetSTC()->DeleteRange(start, GetSTC()->GetCurrentPos() - start);
                }
              }
              break;
            case CHR_TO_NUM('d','$'):
              if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
              {
                if (!GetRegister())
                {
                  GetSTC()->LineEndExtend();
                  GetSTC()->Cut();
                }
                else
                {
                  const int end = GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine());
                  SetRegisterRange(GetSTC()->GetCurrentPos(), end);
                  GetSTC()->DeleteRange(GetSTC()->GetCurrentPos(), end - GetSTC()->GetCurrentPos());
                }
              }
              break;
              
            case CHR_TO_NUM('g','g'): GetSTC()->DocumentStart(); break;
            
            case CHR_TO_NUM('y','w'):
              for (int i = 0; i < repeat; i++) GetSTC()->WordRightEnd();
              for (int j = 0; j < repeat; j++) GetSTC()->WordLeftExtend();
                
              if (!GetRegister())
              {
                GetSTC()->Copy();
              }
              else
              {
                GetMacros().SetRegister(GetRegister(), GetSelectedText());
                GetSTC()->SelectNone();
              }
              break;
            case CHR_TO_NUM('y','y'): wxExAddressRange(this, repeat).Yank(); break;
            case CHR_TO_NUM('z','c'):
            case CHR_TO_NUM('z','o'):
              {
                const int level = GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine());
                const int line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
                  GetSTC()->GetCurrentLine(): GetSTC()->GetFoldParent(GetSTC()->GetCurrentLine());

                if (GetSTC()->GetFoldExpanded(line_to_fold) && rest == "zc")
                  GetSTC()->ToggleFold(line_to_fold);
                else if (!GetSTC()->GetFoldExpanded(line_to_fold) && rest == "zo")
                  GetSTC()->ToggleFold(line_to_fold);
              }
              break;
            case CHR_TO_NUM('z','E'):
            case CHR_TO_NUM('z','f'):
              GetSTC()->SetLexerProperty("fold", (int)rest[1] == 'f' ? "1": "0");
              GetSTC()->Fold(rest[1] == 'f');
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
                case MODE_NORMAL: wxExAddressRange(this, repeat).Indent(rest == ">>"); break;
                case MODE_VISUAL: 
                case MODE_VISUAL_LINE: 
                  wxExAddressRange(this, "'<,'>").Indent(rest == ">>"); break;
              }
              break;
            case CHR_TO_NUM('@','@'): MacroPlayback(GetMacros().GetMacro(), repeat); break;
              
            default:
              if (wxString(rest).Matches("f?") || wxString(rest).Matches("F?"))
              {
                for (int i = 0; i < repeat; i++) 
                  if (!GetSTC()->FindNext(rest.back(), GetSearchFlags(), rest[0] == 'f'))
                    return false;
                m_LastFindCharCommand = command;
              }
              else if (OneLetterAfter("m", rest))
              {
                MarkerAdd(rest.back());
              }
              else if (OneLetterAfter("q", rest))
              {
                if (!GetMacros().IsRecording())
                {
                  MacroStartRecording(rest.substr(1));
                  return true; // as we should not do default actions
                }
              } 
              else if (wxString(rest).Matches("r?"))
              {
                if (!GetSTC()->GetReadOnly())
                {
                  if (GetSTC()->HexMode())
                  {
                    wxExHexModeLine ml(GetSTC());
                  
                    if (ml.IsReadOnly())
                    {
                      return false;
                    }
                  
                    ml.Replace(rest.back());
                  }
                  else
                  {
                    GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
                    GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + repeat);
                    GetSTC()->ReplaceTarget(wxString(rest.back(), repeat));
                  }
                }
              }
              else if (wxString(rest).Matches("t?") || wxString(rest).Matches("T?"))
              {
                for (int i = 0; i < repeat; i++) 
                  if (!GetSTC()->FindNext(rest.back(), GetSearchFlags(), rest[0] == 't'))
                    return false;
                GetSTC()->CharLeft();
                m_LastFindCharCommand = command;
              }
              else if (OneLetterAfter("'", rest))
              {
                MarkerGoto(rest.back());
              }
              else if (OneLetterAfter("@", rest))
              {
                const wxString macro = rest.back();
                
                if (GetMacros().IsRecorded(macro))
                {
                  MacroPlayback(macro, repeat);
                }
                else if (GetMacros().StartsWith(macro))
                {
                  GetFrame()->StatusText(macro, "PaneMacro");
                  return false;
                }
                else
                {
                  m_Command.clear();
                  GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
                }
              }
              else if (RegAfter(wxUniChar(WXK_CONTROL_R), rest))
              {
                CommandReg(rest[1]);
                return true;
              }  
              else if (CommandChar((int)rest[0], repeat))
              {
                rest = rest.substr(1);
              }
              else if (rest.front() == '@')
              {
                std::vector <wxString> v;
                  
                if (wxExMatch("@([a-zA-Z].+)@", rest, v) > 0)
                {
                  handled = MacroPlayback(v[0], repeat);
                  
                  if (!handled)
                  {
                    m_Command.clear();
                    GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
                  }
                }
                else if (GetMacros().StartsWith(rest.substr(1)))
                {
                  wxString s;
                  
                  if (wxExAutoComplete(rest.substr(1), GetMacros().Get(), s))
                  {
                    GetFrame()->StatusText(s, "PaneMacro");
                    
                    handled = MacroPlayback(s, repeat);
                    
                    if (!handled)
                    {
                      m_Command.clear();
                      GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
                    }
                  }
                  else
                  {
                    GetFrame()->StatusText(rest.substr(1), "PaneMacro");
                    return false;
                  }
                }
                else
                {
                  m_Command.clear();
                  GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
                }
              }
              else if (command == ":reg")
              {
                wxString output;
                
                // Currently the " register does not really exist,
                // but copies clipboard contents instead.
                const wxString clipboard(wxExSkipWhiteSpace(wxExClipboardGet()));
              
                if (!clipboard.empty())
                {
                  output += "\": " + clipboard + "\n";
                }
                
                output += "%: " + GetSTC()->GetFileName().GetFullName() + "\n";
                
                for (const auto& it : GetMacros().GetRegisters())
                {
                  output += it + "\n";
                }
              
                if (m_Dialog == NULL)
                {
                  m_Dialog = new wxExSTCEntryDialog(
                    wxTheApp->GetTopWindow(),
                    "Registers", 
                    output,
                    wxEmptyString,
                    wxOK);
                }
                else
                {
                  m_Dialog->GetSTC()->SetText(output);
                }
                
                m_Dialog->Show();
              }
              else if (command == "dgg")
              {
                Command("1,.d");
              }
              else
              {
                handled = false;
              }
            } // switch (CHR_TO_NUM((int)rest[0], (int)rest[1]))
        } // switch (rest.size())
      
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
    if (m_Mode == MODE_VISUAL || m_Mode == MODE_VISUAL_LINE)
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
    wxLogStatus(wxString::Format("%.*f", width, sum));
  }
}

bool wxExVi::CommandChar(int c, int repeat)
{
  switch (c)
  {
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

    // Navigate commands (B, W, E, treated as b, w, e and (,) as {,} for the moment).
    case 'h': 
    case WXK_LEFT:
              NAVIGATE(repeat, Char, Left,     GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0, false); break;
    case 'l': 
    case ' ': 
    case WXK_RIGHT:
              NAVIGATE(repeat, Char, Right,    GetSTC()->GetCurrentPos() < GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine()), false); break;
    case 'j': 
    case WXK_DOWN:
              NAVIGATE(repeat, Line, Down,     GetSTC()->GetCurrentLine() < GetSTC()->GetNumberOfLines(), false); break;
    case 'k': 
    case WXK_UP:
              NAVIGATE(repeat, Line, Up,       GetSTC()->GetCurrentLine() > 0, false); break;
    // does not include wrapped lines
    case '+': 
    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
              NAVIGATE(repeat, Line, Down,     true, true); break;
    case '-': 
              NAVIGATE(repeat, Line, Up,       GetSTC()->GetCurrentLine() > 0, true); break;    
    case 'B':
    case 'b': NAVIGATE(repeat, Word, Left,     true, false); break;
    case 'W':
    case 'w': NAVIGATE(repeat, Word, Right,    true, false); break;
    case 'E':
    case 'e': NAVIGATE(repeat, Word, RightEnd, true, false); break;
    case '(':
    case '{': NAVIGATE(repeat, Para, Up,       true, false); break;
    case ')':
    case '}': NAVIGATE(repeat, Para, Down,     true, false); break;
    case WXK_CONTROL_B:
    case WXK_PAGEUP:
              NAVIGATE(repeat, Page, Up,       true, false); break;
    case WXK_CONTROL_F:
    case WXK_PAGEDOWN:
              NAVIGATE(repeat, Page, Down,     true, false); break;
        
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
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
        for (int i = 0; i < repeat; i++) 
        {
          GetSTC()->CharRightExtend();
        }  
      
        GetSTC()->Cut(); 
      }
      break;
        
    case 'D': 
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
        GetSTC()->LineEndExtend();
        GetSTC()->Cut();
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
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode()) 
      {
        for (int i = 0; i < repeat; i++) 
        {
          GetSTC()->CharLeftExtend();
        }  
      
        GetSTC()->Cut(); 
      }
      break;

    case '.': 
      m_Dot = true;
      Command(GetLastCommand());
      m_Dot = false;
      break;
        
    case ';': 
      m_Dot = true;
      Command(m_LastFindCharCommand); 
      m_Dot = false;
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
      }
      break;
    
    case WXK_CONTROL_E: 
      for (int i = 0; i < repeat; i++) ChangeNumber(true); 
      break;
    case WXK_CONTROL_G:
      GetFrame()->ShowExMessage(wxString::Format("%s line %d of %d --%d%%--", 
        GetSTC()->GetFileName().GetFullName().c_str(), 
        GetSTC()->GetCurrentLine() + 1,
        GetSTC()->GetLineCount(),
        100 * (GetSTC()->GetCurrentLine() + 1)/ GetSTC()->GetLineCount()));
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

void wxExVi::CommandReg(const char reg)
{
  switch (reg)
  {
    case 0: break;
    // calc register
    case '=': GetFrame()->GetExCommand(this, reg); break;
    // clipboard register
    case '\"': Put(true); break;
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
      if (m_Mode == MODE_INSERT)
      {
        if (!GetMacros().GetRegister(reg).empty())
        {
          AddText(GetMacros().GetRegister(reg));
        }
        else
        {
          wxLogStatus("?" + wxString(reg));
        }
      }
      else
      {
        wxLogStatus("?" + wxString(reg));
      }
  }
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
          
  if (brace_match != wxSTC_INVALID_POSITION)
  {
    GetSTC()->GotoPos(brace_match);
  }
  else
  {
    pos = GetSTC()->GetCurrentPos() - 1;
    brace_match = GetSTC()->BraceMatch(pos);
            
    if (brace_match != wxSTC_INVALID_POSITION)
    {
      GetSTC()->GotoPos(brace_match);
    }
  }

  if (m_Mode == MODE_VISUAL)
  {
    if (brace_match != wxSTC_INVALID_POSITION)
    {
      if (brace_match < pos)
        GetSTC()->SetSelection(brace_match, pos + 1);
      else
        GetSTC()->SetSelection(pos, brace_match + 1);
    }
  }
  else if (m_Mode == MODE_VISUAL_LINE)
  {
    if (brace_match != wxSTC_INVALID_POSITION)
    {
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
      if (m_InsertText.size() > 1)
      {
        m_InsertText.resize(m_InsertText.size() - 1);
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
        const std::string lc(GetLastCommand() + m_InsertText);
        
        SetLastCommand(lc + wxString(wxUniChar(WXK_ESCAPE)).ToStdString());
          
        // Record it (if recording is on).
        GetMacros().Record(lc);
        GetMacros().Record(wxString(wxUniChar(WXK_ESCAPE)).ToStdString());
      }
      
      m_Mode = MODE_NORMAL;
      
      GetSTC()->SetOvertype(false);
      
      if (!GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->SelectNone();
      }
      break;

    case WXK_RETURN:
    case WXK_NUMPAD_ENTER:
      GetSTC()->NewLine();
        
      if (!GetSTC()->AutoCompActive())
      {
        m_InsertText += GetSTC()->GetEOL();
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
        m_InsertText += command;
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
  if (!GetSTC()->CanPaste())
  {
    return false;
  }

  if (YankedLines())
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
  
  if (!GetRegister())
  {
    // TODO: Paste does not add NULL.
    // AddText(GetMacros().GetRegister('0'));
    GetSTC()->Paste();
  }
  else
  {
    AddText(GetMacros().GetRegister(GetRegister()));
  }

  if (YankedLines() && after)
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
  m_InsertText.clear();
  
  if (!m_Dot)
  {
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
      GetSTC()->Cut();
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

void  wxExVi::SetRegisterRange(int start, int end)
{
  wxCharBuffer b(GetSTC()->GetTextRangeRaw(start, end));
  
  GetMacros().SetRegister(
    GetRegister(), 
    std::string(b.data(), b.length()));
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

bool wxExVi::YankedLines()
{
  const wxString txt = (!GetRegister() ?
    wxExClipboardGet(): 
    GetMacros().GetRegister(GetRegister()));
  
  // do not trim
  return wxExGetNumberOfLines(txt, false) > 1;
}

#endif // wxUSE_GUI
