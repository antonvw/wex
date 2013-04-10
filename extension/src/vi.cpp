////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/extension/vi.h>
#include <wx/extension/frd.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

// Returns true if after text only one letter is followed.
bool OneLetterAfter(const wxString text, const wxString& letter)
{
  return wxRegEx("^" + text + "[a-zA-Z]$").Matches(letter);
}

bool RegAfter(const wxString text, const wxString& letter)
{
  return wxRegEx("^" + text + "[0-9=\"a-z]$").Matches(letter);
}

wxString wxExVi::m_LastFindCharCommand;

wxExVi::wxExVi(wxExSTC* stc)
  : wxExEx(stc)
  , m_Dot(false)
  , m_InsertMode(false)
  , m_InsertRepeatCount(1)
  , m_SearchForward(true)
{
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

bool wxExVi::Command(const wxString& command)
{
  if (command.empty())
  {
    return false;
  }
  
  if (command.StartsWith("="))
  {
    // Calculation register.
    const wxString calc = command.Mid(1);
    
    wxStringTokenizer tkz(calc, "+-*/");

    double sum = 0;
    bool init = true;
    wxChar prev_cmd = 0;
    int width = 0;

    while (tkz.HasMoreTokens())
    {
      const wxString token = tkz.GetNextToken();
      const int new_width = token.AfterFirst(',').length();
      if (new_width > width) width = new_width;
      const double value = atof(token);
      const wxChar cmd = tkz.GetLastDelimiter();
      
      if (init)
      {
        init = false;
        sum = value;
      }
      else
      {
        switch (prev_cmd)
        {
          case 0: break;
          case '+': sum += value; break;
          case '-': sum -= value; break;
          case '*': sum *= value; break;
          case '/': sum /= value; break;
        }
      }
      
      prev_cmd = cmd;
    }
    
    if (m_InsertMode)
    {
      GetSTC()->AddText(wxString::Format("%.*f", width, sum));
    }
    else
    {
      wxLogStatus(wxString::Format("%.*f", width, sum));
    }
    
    GetFrame()->HideExBar();

    return true;
  }
  else if (m_InsertMode)
  {
    InsertMode(command);
    return true;
  }
  else if (command.StartsWith("/") || command.StartsWith("?"))
  {
    bool result = true;
    
    if (command.length() > 1)
    {
      m_SearchForward = command.StartsWith("/");
        
      // This is a previous entered command.
      result = GetSTC()->FindNext(
        command.Mid(1),
        GetSearchFlags(),
        m_SearchForward);
          
      if (result)
      {
        GetMacros().Record(command);
      }
    }
    else
    {
      GetFrame()->GetExCommand(this, command);
    }
    
    return result;
  }
  
  bool handled = true;

  wxString rest(command);
  
  if (rest.StartsWith("\""))
  {
    if (rest.size() < 2)
    {
      return false;
    }
    
    SetRegister(rest.Mid(1, 1));
    rest = rest.Mid(2);
    
    if (rest.empty())
    {
      return false;
    }
  }
  else
  {
    SetRegister(wxEmptyString);
  }
  
  char * pEnd;
  long int repeat = strtol(rest.c_str(), &pEnd, 10);

  if (repeat == 0)
  {
    repeat++;
  }
  
  rest = wxString(pEnd);
  
  const int size = GetSTC()->GetLength();
  
  if (command == "0")
  {
    GetSTC()->Home(); 
  }
  // Handle multichar commands.
  else if (rest.StartsWith("cc"))
  {
    if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
    {
      GetSTC()->Home();
      GetSTC()->DelLineRight();

      if (!SetInsertMode("cc", repeat))
      {
        return false;
      }
    
      InsertMode(rest.Mid(2));
    }
    
    return true;
  }
  else if (rest.StartsWith("cw"))
  {
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
    
      InsertMode(rest.Mid(2));
    }
    
    return true;
  }
  else if (rest == "dd")
  {
     if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode()) Delete(repeat);
  }
  else if (rest == "de")
  {
    if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
    {
      const int start = GetSTC()->GetCurrentPos();
      for (int i = 0; i < repeat; i++) 
        GetSTC()->WordRightEnd();
        
      if (GetRegister().empty())
      {
        GetSTC()->SetSelection(start, GetSTC()->GetCurrentPos());
        GetSTC()->Cut();
      }
      else
      {
        GetMacros().SetRegister(
          GetRegister(), 
          GetSTC()->GetTextRange(start, GetSTC()->GetCurrentPos()));
        GetSTC()->DeleteRange(start, GetSTC()->GetCurrentPos() - start);
      }
    }
  }
  else if (rest == "d0")
  {
    if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
    {
      if (GetRegister().empty())
      {
        GetSTC()->HomeExtend();
        GetSTC()->Cut();
      }
      else
      {
        const int start = GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine());
        GetMacros().SetRegister(
          GetRegister(), 
          GetSTC()->GetTextRange(start, GetSTC()->GetCurrentPos()));
        GetSTC()->DeleteRange(start, GetSTC()->GetCurrentPos() - start);
      }
    }
  }
  else if (rest == "d$")
  {
    if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
    {
      if (GetRegister().empty())
      {
        GetSTC()->LineEndExtend();
        GetSTC()->Cut();
      }
      else
      {
        const int end = GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine());
        GetMacros().SetRegister(
          GetRegister(), 
          GetSTC()->GetTextRange(GetSTC()->GetCurrentPos(), end));
        GetSTC()->DeleteRange(GetSTC()->GetCurrentPos(), end - GetSTC()->GetCurrentPos());
      }
    }
  }
  else if (rest == "dw")
  {
    if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
    {
      const int start = GetSTC()->GetCurrentPos();
      for (int i = 0; i < repeat; i++) 
        GetSTC()->WordRight();
        
      if (GetRegister().empty())
      {
        GetSTC()->SetSelection(start, GetSTC()->GetCurrentPos());
        GetSTC()->Cut();
      }
      else
      {
        GetMacros().SetRegister(
          GetRegister(), 
          GetSTC()->GetTextRange(start, GetSTC()->GetCurrentPos()));
        GetSTC()->DeleteRange(start, GetSTC()->GetCurrentPos() - start);
      }
    }
  }
  else if (rest.Matches("f?"))
  {
    for (int i = 0; i < repeat; i++) 
      GetSTC()->FindNext(rest.Last(), GetSearchFlags());
    m_LastFindCharCommand = command;
  }
  else if (rest.Matches("F?"))
  {
    for (int i = 0; i < repeat; i++) 
      GetSTC()->FindNext(rest.Last(), GetSearchFlags(), false);
    m_LastFindCharCommand = command;
  }
  else if (OneLetterAfter("m", rest))
  {
    MarkerAdd(rest.Last());
  }
  else if (rest.Matches("r?"))
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
      
        ml.Replace(rest.Last());
      }
      else
      {
        GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
        GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + repeat);
        GetSTC()->ReplaceTarget(wxString(rest.Last(), repeat));
      }
    }
  }
  else if (rest == "yw")
  {
    for (int i = 0; i < repeat; i++) 
      GetSTC()->WordRightEnd();
    for (int j = 0; j < repeat; j++) 
      GetSTC()->WordLeftExtend();
      
    if (GetRegister().empty())
    {
      GetSTC()->Copy();
    }
    else
    {
      GetMacros().SetRegister(
        GetRegister(),
        GetSTC()->GetSelectedText());
    }
  }
  else if (rest == "yy")
  {
    Yank(repeat);
  }
  else if (rest == "zc" || rest == "zo")
  {
    const int level = GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine());
    const int line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetSTC()->GetCurrentLine(): GetSTC()->GetFoldParent(GetSTC()->GetCurrentLine());

    if (GetSTC()->GetFoldExpanded(line_to_fold) && rest == "zc")
      GetSTC()->ToggleFold(line_to_fold);
    else if (!GetSTC()->GetFoldExpanded(line_to_fold) && rest == "zo")
      GetSTC()->ToggleFold(line_to_fold);
  }
  else if (rest == "zE")
  {
    GetSTC()->SetLexerProperty("fold", "0");
    GetSTC()->Fold();
  }
  else if (rest == "zf")
  {
    GetSTC()->SetLexerProperty("fold", "1");
    GetSTC()->Fold(true);
  }
  else if (rest == "ZZ")
  {
    wxPostEvent(wxTheApp->GetTopWindow(), 
      wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
    wxPostEvent(wxTheApp->GetTopWindow(), 
      wxCloseEvent(wxEVT_CLOSE_WINDOW));
  }
  else if (rest == ">>")
  {
    GetSTC()->Indent(repeat);
  }
  else if (rest == "<<")
  {
    GetSTC()->Indent(repeat, false);
  }
  else if (OneLetterAfter("'", rest))
  {
    MarkerGoto(rest.Last());
  }
  else if (OneLetterAfter("q", rest))
  {
    if (!GetMacros().IsRecording())
    {
      MacroStartRecording(rest.Mid(1));
      return true; // as we should not do default actions
    }
  } 
  else if (rest == "@@")
  {
    MacroPlayback(GetMacros().GetMacro(), repeat);
  }
  else if (OneLetterAfter("@", rest))
  {
    const wxString macro = rest.Last();
    
    if (GetMacros().IsRecorded(macro))
    {
      MacroPlayback(macro, repeat);
    }
    else
    {
      GetFrame()->StatusText(macro, "PaneMacro");
      return false;
    }
  }
  else if (RegAfter(wxUniChar(WXK_CONTROL_R), rest))
  {
    CommandReg(rest.Mid(1));
  }  
  else if (rest.StartsWith("@"))
  {
    std::vector <wxString> v;
      
    if (wxExMatch("@(.+)@", rest, v) > 0)
    {
      const wxString macro = v[0];
        
      if (GetMacros().IsRecorded(macro))
      {
        handled = MacroPlayback(macro, repeat);
      }
      else
      {
        handled = GetMacros().Expand(this, macro);
      }
      
      if (!handled)
      {
        m_Command.clear();
        GetFrame()->StatusText(GetMacros().GetMacro(), "PaneMacro");
      }
    }
    else
    {
      GetFrame()->StatusText(rest.Mid(1), "PaneMacro");
      return false;
    }
  }
  else if (!rest.empty())
  {
    // Handle ESCAPE, should clear command buffer,
    // as last char, so not in switch branch.
    if (!m_Dot && rest.Last() == WXK_ESCAPE)
    {
      wxBell();
        
      m_Command.clear();

      if (GetMacros().IsRecording())
      {
        GetMacros().StopRecording();
      }
      
      if (!GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->SelectNone();
      }
    }
    else
    {
      handled = CommandChar((int)rest.GetChar(0), repeat);
        
      if (m_InsertMode)
      {
        InsertMode(rest.Mid(1));
        return true;
      }
    }
  }
  else
  {
    handled = false;
  }

  if (!handled)
  {  
    return wxExEx::Command(command);
  }
  
  if (!m_Dot)
  {
    // Set last command.
    SetLastCommand(command, 
      // Always when in insert mode,
      // or this was a file change command (so size different from before).
      m_InsertMode ||
      size != GetSTC()->GetLength());

    // Record it (if recording is on).
    GetMacros().Record(command);
  }
    
  return true;
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
        
    case 'b': for (int i = 0; i < repeat; i++) GetSTC()->WordLeft(); break;

    case 'e': for (int i = 0; i < repeat; i++) GetSTC()->WordRightEnd(); break;
      
    case 'g': GetSTC()->DocumentStart(); break;
      
    case 'h': 
      for (int i = 0; i < repeat; i++) 
      {
        if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0)
        {
          GetSTC()->CharLeft(); 
        }
      }
      break;
        
    case 'j': 
      for (int i = 0; i < repeat; i++) 
      {
         if (GetSTC()->GetCurrentLine() < GetSTC()->GetNumberOfLines())
         {
           GetSTC()->LineDown(); 
         }
      }
      break;
        
    case '+': 
      for (int i = 0; i < repeat; i++) 
      {
        GetSTC()->LineDown();
        
        for (int j = 1; j < GetSTC()->WrapCount(GetSTC()->GetCurrentLine()); j++)
        {
          GetSTC()->LineDown();
        }
      }
      
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) != 
          GetSTC()->GetLineIndentation(GetSTC()->GetCurrentLine()))
      {
        GetSTC()->VCHome();
      }
      break;
        
    case 'k': 
      for (int i = 0; i < repeat; i++) 
      {
        if (GetSTC()->GetCurrentLine() > 0)
        {
          GetSTC()->LineUp(); 
        }
      }
      break;
        
    case '-': 
      for (int i = 0; i < repeat; i++) GetSTC()->LineUp(); 
      
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) != 
          GetSTC()->GetLineIndentation(GetSTC()->GetCurrentLine()))
      {
        GetSTC()->VCHome();
      }
      break;
        
    case 'l': 
    case ' ': 
      for (int i = 0; i < repeat; i++) 
      {
        if (GetSTC()->GetCurrentPos() < 
            GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine()))
        {
          GetSTC()->CharRight(); 
        }
      }
      break;
        
    case 'n': 
      for (int i = 0; i < repeat; i++) 
        if (!GetSTC()->FindNext(
          wxExFindReplaceData::Get()->GetFindString(), 
          GetSearchFlags(), 
          m_SearchForward)) break;
      break;

    case 'p': 
      Put(true); 
      break;
      
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
      
    case 'w': for (int i = 0; i < repeat; i++) GetSTC()->WordRight(); break;
      
    case 'x': 
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
        for (int i = 0; i < repeat; i++) 
        {
          GetSTC()->CharRight();
          GetSTC()->DeleteBack(); 
        }  
      }
      break;
        
    case 'y': 
      if (GetSTC()->CanCopy())
      {
        GetSTC()->Copy();
      } 
      else
      {
        return false;
      }
      break;

    case 'D': 
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
      {
        GetSTC()->LineEndExtend();
        GetSTC()->Cut();
      }
      break;
        
    case 'G': 
      if (repeat > 0)
      {
        GetSTC()->GotoLineAndSelect(repeat);
      }
      else
      {
        GetSTC()->DocumentEnd();
      }
      break;
        
    case 'H': GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine());
      break;
        
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
        
    case 'P': 
      Put(false); 
      break;
      
    case 'X': 
      if (!GetSTC()->GetReadOnly() && !GetSTC()->HexMode()) 
        for (int i = 0; i < repeat; i++) GetSTC()->DeleteBack();
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
        
    case '^': GetSTC()->Home(); break;
    case '~': return ToggleCase(); break;
    case '$': GetSTC()->LineEnd(); break;
    case '{': for (int i = 0; i < repeat; i++) GetSTC()->ParaUp(); break;
    case '}': for (int i = 0; i < repeat; i++) GetSTC()->ParaDown(); break;
    case '%': GotoBrace(); break;

    case '*': FindWord(); break;
    case '#': FindWord(false); break;
      
    case WXK_CONTROL_B:
      for (int i = 0; i < repeat; i++) GetSTC()->PageUp(); 
      break;
    case WXK_CONTROL_E: 
      for (int i = 0; i < repeat; i++) ChangeNumber(true); 
      break;
    case WXK_CONTROL_F:
      for (int i = 0; i < repeat; i++) GetSTC()->PageDown(); 
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
      
    case WXK_RETURN:
      for (int i = 0; i < repeat; i++) GetSTC()->LineDown();
      break;
    
    case WXK_TAB:
      // just ignore tab
      break;
      
    default:
      return false;
  }
  
  return true;
}

void wxExVi::CommandReg(const wxString& reg)
{
  if (reg == "=")
  {
    GetFrame()->GetExCommand(this, reg);
  }
  else if (reg == "\"")
  {
    Put(true);
  }
  else if (m_InsertMode)
  {
    if (!GetMacros().GetRegister(reg).empty())
    {
      GetSTC()->AddText(GetMacros().GetRegister(reg));
    }
    else
    {
      wxLogStatus("?" + reg);
    }
  }
  else
  {
    wxLogStatus("?" + reg);
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
  int brace_match = GetSTC()->BraceMatch(GetSTC()->GetCurrentPos());
          
  if (brace_match != wxSTC_INVALID_POSITION)
  {
    GetSTC()->GotoPos(brace_match);
  }
  else
  {
    brace_match = GetSTC()->BraceMatch(GetSTC()->GetCurrentPos() - 1);
            
    if (brace_match != wxSTC_INVALID_POSITION)
    {
      GetSTC()->GotoPos(brace_match);
    }
  }
}

void wxExVi::InsertMode(const wxString& command)
{
  if (command.empty())
  {
    return;
  }
  
  switch ((int)command.Last())
  {
    case WXK_BACK:
        if (m_InsertText.size() > 1)
        {
          m_InsertText.Truncate(m_InsertText.size() - 1);
        }
        
        GetSTC()->DeleteBack();
      break;
      
    case WXK_ESCAPE:
        // Add extra inserts if necessary.        
        if (!m_InsertText.empty())
        {
          for (int i = 1; i < m_InsertRepeatCount; i++)
          {
            GetSTC()->AddText(m_InsertText);
          }
        }
        
        // If we have text to be added.
        if (command.size() > 1)
        { 
          const wxString rest(command.Left(command.size() - 1));
          
          if (!GetSTC()->GetSelectedText().empty())
          {
            GetSTC()->ReplaceSelection(rest);
          }
          else
          {
            for (int i = 1; i <= m_InsertRepeatCount; i++)
            {
              GetSTC()->AddText(rest);
            }
          }
        }
          
        GetSTC()->EndUndoAction();
        
        if (!m_Dot)
        {
          const wxString lc(GetLastCommand() + m_InsertText);
          
          SetLastCommand(lc + wxUniChar(WXK_ESCAPE));
            
          // Record it (if recording is on).
          GetMacros().Record(lc);
          GetMacros().Record(wxUniChar(WXK_ESCAPE));
        }
        
        m_InsertMode = false;
      break;

    case WXK_RETURN:
        m_InsertText += GetSTC()->GetEOL();
        GetSTC()->NewLine();
      break;
      
    case WXK_CONTROL_R:
      m_InsertText += command;
      break;
    
    default: 
      if (GetLastCommand().EndsWith("cw") && m_InsertText.empty())
      {
        GetSTC()->ReplaceSelection(wxEmptyString);
      }

      if (
       !m_InsertText.empty() &&
        m_InsertText.Last() == wxUniChar(WXK_CONTROL_R))
      {
        CommandReg(command);
      }
      else
      {
        GetSTC()->AddText(command);
      }
      
      if (!m_Dot)
      {
        m_InsertText += command;
      }
  }
}

bool wxExVi::OnChar(const wxKeyEvent& event)
{
  if (!GetIsActive())
  {
    return true;
  }
  else if (m_InsertMode)
  {
    InsertMode(event.GetUnicodeKey());
    
    return false;
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
        if (m_Command.StartsWith("@") && event.GetKeyCode() == WXK_BACK)
        {
          m_Command = m_Command.Truncate(m_Command.size() - 1);
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
  if (!GetIsActive())
  {
    return true;
  }
  else if (
    event.GetKeyCode() == WXK_BACK ||
    event.GetKeyCode() == WXK_ESCAPE ||
    event.GetKeyCode() == WXK_RETURN ||
    event.GetKeyCode() == WXK_TAB)
  {
    if (m_Command.StartsWith("@"))
    {
      if (event.GetKeyCode() == WXK_BACK)
      {
        m_Command = m_Command.Truncate(m_Command.size() - 1);
        GetFrame()->StatusText(m_Command.Mid(1), "PaneMacro");
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
      m_Command += event.GetKeyCode();
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
    if (GetMacros().IsRecording() && !m_InsertMode)
    {
      switch (event.GetKeyCode())
      {
        case WXK_LEFT: GetMacros().Record("h"); break;
        case WXK_DOWN: GetMacros().Record("j"); break;
        case WXK_UP: GetMacros().Record("k"); break;
        case WXK_RIGHT: GetMacros().Record("l"); break;
      }
    }
    
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
    if (after) GetSTC()->LineDown();
    GetSTC()->Home();
  }
  
  if (GetRegister().empty())
  {
    GetSTC()->Paste();
  }
  else
  {
    GetSTC()->AddText(GetMacros().GetRegister(GetRegister()));
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
    
  m_InsertMode = true;
  m_InsertText.clear();
  
  if (!m_Dot)
  {
    if (repeat > 1)
    {
      SetLastCommand(wxString::Format("%d%s", repeat, c.c_str()), true);
    }
    else
    {
      SetLastCommand(c, true);
    }
  }
  
  GetSTC()->BeginUndoAction();
  
  switch ((int)c.GetChar(0))
  {
    case 'a': GetSTC()->CharRight(); 
      m_InsertRepeatCount = repeat;
      break;

    case 'c': 
      break;
      
    case 'i': 
      m_InsertRepeatCount = repeat;
      break;

    case 'o': 
      m_InsertRepeatCount = repeat;
      GetSTC()->LineEnd(); 
      GetSTC()->NewLine(); 
      break;
      
    case 'A': GetSTC()->LineEnd(); 
      m_InsertRepeatCount = repeat;
      break;

    case 'C': 
    case 'R': 
      m_InsertRepeatCount = repeat;
      GetSTC()->SetSelectionStart(GetSTC()->GetCurrentPos());
      GetSTC()->SetSelectionEnd(GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine()));
      break;

    case 'I': 
      m_InsertRepeatCount = repeat;
      GetSTC()->Home(); 
      break;

    case 'O': 
      m_InsertRepeatCount = repeat;
      GetSTC()->Home(); 
      GetSTC()->NewLine(); 
      GetSTC()->LineUp(); 
      break;

    default: wxFAIL;
  }

  GetSTC()->SetOvertype((int)c.GetChar(0) == 'R');
  
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

bool wxExVi::YankedLines()
{
  const wxString txt = (GetRegister().empty() ?
    wxExClipboardGet(): 
    GetMacros().GetRegister(GetRegister()));
  
  // do not trim
  return wxExGetNumberOfLines(txt, false) > 1;
}

#endif // wxUSE_GUI
