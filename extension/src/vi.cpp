////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/regex.h>
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
    
    GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
    
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
  
  if (m_InsertMode)
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
        MacroRecord(command);
      }
    }
    else
    {
      GetFrame()->GetExCommand(this, command);
    }
    
    return result;
  }
  
  bool handled = true;

  char * pEnd;
  long int repeat = strtol(command.c_str(), &pEnd, 10);

  if (repeat == 0)
  {
    repeat++;
  }
  
  const int size = GetSTC()->GetLength();
  
  const wxString rest(pEnd);
  
  if (command == "0")
  {
    GetSTC()->Home(); 
  }
  // Handle multichar commands.
  else if (rest.StartsWith("cc") && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->Home();
    GetSTC()->DelLineRight();

    if (!SetInsertMode("cc", repeat))
    {
      return false;
    }
    
    InsertMode(rest.Mid(2));
    return true;
  }
  else if (rest.StartsWith("cw") && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
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
    return true;
  }
  else if (rest == "dd")
  {
    return Delete(repeat);
  }
  else if (rest == "d0" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->HomeExtend();
    GetSTC()->Cut();
  }
  else if (rest == "d$" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->LineEndExtend();
    GetSTC()->Cut();
  }
  else if (rest == "dw" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->BeginUndoAction();
    const int start = GetSTC()->GetCurrentPos();
    for (int i = 0; i < repeat; i++) 
      GetSTC()->WordRight();
    GetSTC()->SetSelection(start, GetSTC()->GetCurrentPos());
    GetSTC()->Cut();
    GetSTC()->EndUndoAction();
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
  else if (rest.Matches("r?") && !GetSTC()->GetReadOnly())
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
      GetSTC()->MarkTargetChange();
    }
  }
  else if (rest == "yw")
  {
    for (int i = 0; i < repeat; i++) 
      GetSTC()->WordRightEnd();
    for (int j = 0; j < repeat; j++) 
      GetSTC()->WordLeftExtend();
    GetSTC()->Copy();
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
  else if (rest == ">>" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->Indent(repeat);
  }
  else if (rest == "<<" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->Indent(repeat, false);
  }
  else if (OneLetterAfter("'", rest))
  {
    MarkerGoto(rest.Last());
  }
  else if (OneLetterAfter("q", rest))
  {
    if (!MacroIsRecording())
    {
      MacroStartRecording(rest.Mid(1));
      return true; // as we should not do default actions
    }
  } 
  else if (rest == "@@")
  {
    MacroPlayback(GetMacro(), repeat);
  }
  else if (OneLetterAfter("@", rest))
  {
    if (MacroIsRecorded(rest.Last()))
    {
      MacroPlayback(rest.Last(), repeat);
    }
    else
    {
      return false;
    }
  }
  else if (rest.StartsWith("@"))
  {
    std::vector <wxString> v;
    
    if (wxExMatch("@(.+)@", rest, v) > 0)
    {
      if (MacroIsRecorded(v[0]))
      {
        handled = MacroPlayback(v[0], repeat);
      }
      else
      {
        handled = MacroExpand(v[0]);
      }
      
      if (!handled)
      {
        m_Command.clear();
      }
    }
    else
    {
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

      if (!GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->SetSelection(
          GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos());
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
    MacroRecord(command);
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
      return SetInsertMode((char)c, repeat); 
      break;
        
    case 'b': for (int i = 0; i < repeat; i++) GetSTC()->WordLeft(); break;

    case 'e': for (int i = 0; i < repeat; i++) GetSTC()->WordRightEnd(); break;
      
    case 'g': GetSTC()->DocumentStart(); break;
      
    case 'h': 
      for (int i = 0; i < repeat; i++) GetSTC()->CharLeft(); 
      break;
        
    case 'j': 
      for (int i = 0; i < repeat; i++) GetSTC()->LineDown(); 
      break;
        
    case 'k': 
      for (int i = 0; i < repeat; i++) GetSTC()->LineUp(); 
      break;
        
    case 'l': 
    case ' ': 
      for (int i = 0; i < repeat; i++) GetSTC()->CharRight(); 
      break;
        
    case 'n': 
      for (int i = 0; i < repeat; i++) 
        if (!GetSTC()->FindNext(
          wxExFindReplaceData::Get()->GetFindString(), 
          GetSearchFlags(), 
          m_SearchForward)) break;
      break;

    case 'p': 
      return Put(true); 
      break;
      
    case 'q': 
      if (MacroIsRecording())
      {
        MacroStopRecording();
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
      if (GetSTC()->GetReadOnly() || GetSTC()->HexMode()) return false;
      
      for (int i = 0; i < repeat; i++) 
      {
        GetSTC()->CharRight();
        GetSTC()->DeleteBack(); 
      }
      GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
      break;
        
    case 'y': 
      if (!GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->Copy();
      } 
      else
      {
        return false;
      }
      break;

    case 'D': 
      if (GetSTC()->GetReadOnly() || GetSTC()->HexMode()) return false;
      GetSTC()->LineEndExtend();
      GetSTC()->Cut();
      break;
        
    case 'G': 
      if (repeat > 1)
      {
        GetSTC()->GotoLine(repeat - 1);
      }
      else
      {
        GetSTC()->DocumentEnd();
      }
      break;
        
    case 'H': GetSTC()->GotoLine(GetSTC()->GetFirstVisibleLine());
      break;
        
    case 'J':
      if (GetSTC()->GetReadOnly() || GetSTC()->HexMode()) return false;
      GetSTC()->BeginUndoAction();
      GetSTC()->SetTargetStart(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()));
      GetSTC()->SetTargetEnd(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine() + repeat));
      GetSTC()->LinesJoin();
      GetSTC()->EndUndoAction();
      break;
        
    case 'L': GetSTC()->GotoLine(
      GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen()); 
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
      return Put(false); 
      break;
      
    case 'X': 
      if (GetSTC()->GetReadOnly() || GetSTC()->HexMode()) return false;
      for (int i = 0; i < repeat; i++) GetSTC()->DeleteBack();
      GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
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
      if (GetSTC()->GetReadOnly() || GetSTC()->HexMode()) return false;
      GetSTC()->DeleteBack();
      GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
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
          MacroRecord(lc);
          MacroRecord( wxUniChar(WXK_ESCAPE));
        }
        
        m_InsertMode = false;
      break;

    case WXK_RETURN:
        m_InsertText += command.Last();

        if (!GetSTC()->SmartIndentation())
        {
          GetSTC()->NewLine();
        }
      break;
    
    default: 
      if (GetLastCommand().EndsWith("cw") && m_InsertText.empty())
      {
        GetSTC()->ReplaceSelection(wxEmptyString);
      }
      
      GetSTC()->AddText(command);
      
      if (!m_Dot)
      {
        m_InsertText += command;
      }
  }
      
  if (GetSTC()->GetModify())
  {
    GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
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
    if (m_Command.StartsWith("@") && event.GetKeyCode() == WXK_BACK)
    {
      m_Command = m_Command.Truncate(m_Command.size() - 1);
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
    if (MacroIsRecording() && !m_InsertMode)
    {
      switch (event.GetKeyCode())
      {
        case WXK_LEFT: MacroRecord("h"); break;
        case WXK_DOWN: MacroRecord("j"); break;
        case WXK_UP: MacroRecord("k"); break;
        case WXK_RIGHT: MacroRecord("l"); break;
      }
    }
    
    return true;
  }
}

bool wxExVi::Put(bool after)
{
  if (GetSTC()->GetReadOnly() || GetSTC()->HexMode())
  {
    return false;
  }
  
  if (YankedLines())
  {
    if (after) GetSTC()->LineDown();
    GetSTC()->Home();
  }

  GetSTC()->Paste();

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
  
  const int line = GetSTC()->LineFromPosition(GetSTC()->GetCurrentPos());
  GetSTC()->MarkerAddChange(line);
  
  return true;
}

bool wxExVi::YankedLines()
{
  const wxString cb(wxExClipboardGet());
  
  // do not trim
  return wxExGetNumberOfLines(cb, false) > 1;
}

#endif // wxUSE_GUI
