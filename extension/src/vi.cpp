////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/tokenzr.h>
#include <wx/extension/vi.h>
#include <wx/extension/frd.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

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
    if (inc)
    {
      GetSTC()->wxStyledTextCtrl::Replace(start, end, 
        wxString::Format("%d", ++number));
    }
    else
    {
      GetSTC()->wxStyledTextCtrl::Replace(start, end, 
        wxString::Format("%d", --number));
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
    return InsertMode(command);
  }
  else
  {
    if (command.StartsWith(":"))
    { 
      if (command.length() > 1)
      {
        // This is a previous entered command.
        return wxExEx::Command(command);
      }
      else
      {
        GetFrame()->GetExCommand(this, command);
        return true;
      }
    }
    else if (command.StartsWith("/") || command.StartsWith("?"))
    {
      if (command.length() > 1)
      {
        // This is a previous entered command.
        const bool result = GetSTC()->FindNext(
          command.Mid(1),
          GetSearchFlags(),
          command.StartsWith("/"));
          
        if (result)
        {
          MacroRecord(command);
        }
        
        return result;
      }
      else
      {
        GetFrame()->GetExCommand(this, command);
        return true;
      }
    }
  }
  
  bool handled = true;

  int repeat = atoi(command.c_str());

  if (repeat == 0)
  {
    repeat++;
  }
  
  // Handle multichar commands.
  if (command.EndsWith("cw") && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    if (!GetSTC()->GetSelectedText().empty())
    {
      GetSTC()->SetCurrentPos(GetSTC()->GetSelectionStart());
    }

    const int pos = GetSTC()->GetCurrentPos();
    
    for (int i = 0; i < repeat; i++) GetSTC()->WordRightEndExtend();

    if (m_Dot)
    {
      GetSTC()->ReplaceSelection(m_InsertText);
    }
    else
    {
      SetInsertMode();
      const int anchor = GetSTC()->GetCurrentPos();
      GetSTC()->SetCurrentPos(pos);
      GetSTC()->SetAnchor(anchor);
      
      if (MacroIsPlayback())
      {
        GetSTC()->ReplaceSelection(wxEmptyString);
      }
    }
  }
  else if (command == "cc" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->Home();
    GetSTC()->DelLineRight();

    if (m_Dot && !m_InsertText.empty())
    {
      GetSTC()->ReplaceSelection(m_InsertText);
    }
    else
    {
      SetInsertMode();
    }
  }
  else if (command.EndsWith("dd") && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    Delete(repeat);
  }
  else if (command == "d0" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->HomeExtend();
    GetSTC()->Cut();
  }
  else if (command == "d$" && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->LineEndExtend();
    GetSTC()->Cut();
  }
  else if (command.EndsWith("dw") && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->BeginUndoAction();
    const int start = GetSTC()->GetCurrentPos();
    for (int i = 0; i < repeat; i++) 
      GetSTC()->WordRight();
    GetSTC()->SetSelection(start, GetSTC()->GetCurrentPos());
    GetSTC()->Cut();
    GetSTC()->EndUndoAction();
  }
  // this one should be first, so rJ will match
  else if (wxRegEx("[0-9]*r.").Matches(command) && !GetSTC()->GetReadOnly())
  {
    if (GetSTC()->HexMode())
    {
      wxExHexModeLine ml(GetSTC());
      
      if (ml.IsReadOnly())
      {
        return false;
      }
      
      ml.Replace(command.Last());
    }
    else
    {
      GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
      GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + repeat);
      GetSTC()->ReplaceTarget(wxString(command.Last(), repeat));
      GetSTC()->MarkTargetChange();
    }
  }
  else if (command.Matches("*f?"))
  {
    for (int i = 0; i < repeat; i++) 
      GetSTC()->FindNext(command.Last(), GetSearchFlags());
    m_LastFindCharCommand = command;
  }
  else if (command.Matches("*F?"))
  {
    for (int i = 0; i < repeat; i++) 
      GetSTC()->FindNext(command.Last(), GetSearchFlags(), false);
    m_LastFindCharCommand = command;
  }
  else if (command.Matches("*J"))
  {
    GetSTC()->BeginUndoAction();
    GetSTC()->SetTargetStart(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine()));
    GetSTC()->SetTargetEnd(GetSTC()->PositionFromLine(GetSTC()->GetCurrentLine() + repeat));
    GetSTC()->LinesJoin();
    GetSTC()->EndUndoAction();
  }
  else if (command.Matches("m?"))
  {
    MarkerAdd(command.Last());
  }
  else if (command.EndsWith("yw"))
  {
    for (int i = 0; i < repeat; i++) 
      GetSTC()->WordRightEnd();
    for (int j = 0; j < repeat; j++) 
      GetSTC()->WordLeftExtend();
    GetSTC()->Copy();
  }
  else if (command.EndsWith("yy"))
  {
    Yank(repeat);
  }
  else if (command == "zc" || command == "zo")
  {
    const int level = GetSTC()->GetFoldLevel(GetSTC()->GetCurrentLine());
    const int line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetSTC()->GetCurrentLine(): GetSTC()->GetFoldParent(GetSTC()->GetCurrentLine());

    if (GetSTC()->GetFoldExpanded(line_to_fold) && command == "zc")
      GetSTC()->ToggleFold(line_to_fold);
    else if (!GetSTC()->GetFoldExpanded(line_to_fold) && command == "zo")
      GetSTC()->ToggleFold(line_to_fold);
  }
  else if (command == "zE")
  {
    GetSTC()->SetLexerProperty("fold", "0");
    GetSTC()->Fold();
  }
  else if (command == "zf")
  {
    GetSTC()->SetLexerProperty("fold", "1");
    GetSTC()->Fold(true);
  }
  else if (command == "ZZ")
  {
    wxPostEvent(wxTheApp->GetTopWindow(), 
      wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
    wxPostEvent(wxTheApp->GetTopWindow(), 
      wxCloseEvent(wxEVT_CLOSE_WINDOW));
  }
  else if (command.EndsWith(">>") && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->Indent(repeat);
  }
  else if (command.EndsWith("<<") && !GetSTC()->GetReadOnly() && !GetSTC()->HexMode())
  {
    GetSTC()->Indent(repeat, false);
  }
  else if (command.Matches("'?"))
  {
    MarkerGoto(command.Last());
  }
  else if (command.Matches("q?"))
  {
    if (!MacroIsRecording())
    {
      MacroStartRecording(command.Mid(1));
      m_Command.clear();
      return false;
    }
  } 
  else if (command.Matches("*@@"))
  {
    MacroPlayback(GetMacro(), repeat);
  }
  else if (command.Matches("*@?"))
  {
    MacroPlayback(command.AfterFirst('@'), repeat);
  }
  else
  {
    switch ((int)command.Last())
    {
      case 'a': 
      case 'i': 
      case 'o': 
      case 'A': 
      case 'C': 
      case 'I': 
      case 'O': 
        SetInsertMode(command.Last(), repeat, false, m_Dot); 
        break;
        
      case 'R': 
        SetInsertMode(command.Last(), repeat, true, m_Dot); 
        break;

      case '0': 
      case '^': 
        if (command.length() == 1)
        {
          GetSTC()->Home(); 
        }
        else
        {
          handled = false;
        }
        break;
        
      case 'b': for (int i = 0; i < repeat; i++) GetSTC()->WordLeft(); break;

      case 'd':
        if (
          !GetSTC()->GetSelectedText().empty() &&
          !GetSTC()->GetReadOnly() && 
          !GetSTC()->HexMode()) 
        {
          GetSTC()->Clear(); 
          GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
        }
        else
        {
          handled = false;
        }
        break;

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

      case 'p': Put(true); break;
      
      case 'q': 
        if (MacroIsRecording())
        {
          MacroStopRecording();
        }
        else
        {
          handled = false;
        }
        break;
      
      case 'u': GetSTC()->Undo(); break;
      
      case 'w': for (int i = 0; i < repeat; i++) GetSTC()->WordRight(); break;
      
      case 'x': 
        if (GetSTC()->HexMode()) return false;
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
          handled = false;
        }
        break;

      case 'D': 
        if (!GetSTC()->GetReadOnly())
        {
          GetSTC()->LineEndExtend();
          GetSTC()->Cut();
          }
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
        
      case 'P': Put(false); break;
      
      case 'X': 
        if (GetSTC()->HexMode()) return false;
        for (int i = 0; i < repeat; i++) GetSTC()->DeleteBack(); break;

      case '.': 
        m_Dot = true;
        Command(GetLastCommand());
        m_Dot = false;
        break;
        
      case ';': Command(m_LastFindCharCommand); break;
      case '~': ToggleCase(); break;
      case '$': GetSTC()->LineEnd(); break;
      case '{': GetSTC()->ParaUp(); break;
      case '}': GetSTC()->ParaDown(); break;
      case '%': GotoBrace(); break;

      case '*': FindWord(); break;
      case '#': FindWord(false); break;
      
      case WXK_CONTROL_B:
        for (int i = 0; i < repeat; i++) GetSTC()->PageUp(); 
        break;
      case WXK_CONTROL_E: 
        for (int i = 0; i < repeat; i++) ChangeNumber(true); 
        break;
      case WXK_CONTROL_G:  // (^f is not possible, already find accel key)
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
        GetSTC()->CharLeft();
        break;
      
      case WXK_ESCAPE:
        wxBell();

        if (!GetSTC()->GetSelectedText().empty())
        {
          GetSTC()->SetSelection(
            GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos());
        }
        break;

      case WXK_RETURN:
        for (int i = 0; i < repeat; i++) GetSTC()->LineDown();
        break;
    
      case WXK_TAB:
        if (m_Command.size() > 0 && wxRegEx("[0-9]*r").Matches(m_Command))
        {
          GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
          GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + repeat);
          GetSTC()->ReplaceTarget(wxString('\t', repeat));
          GetSTC()->MarkTargetChange();
        }
        break;
      
      default:
        handled = false;
    }
  }

  if (handled)
  {  
    if (!m_Dot)
    {
      SetLastCommand(command);
    }
    
    MacroRecord(command);
  }
 
  return handled;
}

void wxExVi::FindWord(bool find_next)
{
  const int start = GetSTC()->WordStartPosition(GetSTC()->GetCurrentPos(), true);
  const int end = GetSTC()->WordEndPosition(GetSTC()->GetCurrentPos(), true);
  
  wxExFindReplaceData::Get()->SetFindString(GetSTC()->GetTextRange(start, end));  
    
  GetSTC()->FindNext(
    wxExFindReplaceData::Get()->GetFindString(), 
    GetSearchFlags() | wxSTC_FIND_WHOLEWORD, 
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

bool wxExVi::InsertMode(const wxString& command)
{
  switch ((int)command.Last())
  {
    case WXK_BACK:
        if (m_InsertText.size() > 1)
        {
          m_InsertText.Truncate(m_InsertText.size() - 1);
        }
        
        GetSTC()->CharLeft();
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
        
        GetSTC()->EndUndoAction();
        
        m_InsertMode = false;
      break;

    case WXK_RETURN:
        m_InsertText += command.Last();

        if (!GetSTC()->SmartIndentation())
        {
          GetSTC()->NewLine();
        }
      break;
    
    default: GetSTC()->AddText(command);
  }
      
  GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
  
  MacroRecord(command);
  
  return true;
}

bool wxExVi::OnChar(const wxKeyEvent& event)
{
  if (!GetIsActive())
  {
    return true;
  }
  else if (m_InsertMode)
  {
    if (MacroIsRecording())
    {
      // Record as new record if insert text is empty,
      // otherwise append it.
      MacroRecord(event.GetUnicodeKey(), m_InsertText.empty());
    }
    
    m_InsertText += event.GetUnicodeKey();
    
    return true;
  }
  else
  {
    if (!(event.GetModifiers() & wxMOD_ALT))
    {
      m_Command += event.GetUnicodeKey();
      
      if (Command(m_Command))
      {
        m_Command.clear();
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
    event.GetKeyCode() == WXK_CONTROL_J ||
    event.GetKeyCode() == WXK_CONTROL_E ||
    event.GetKeyCode() == WXK_ESCAPE ||
    event.GetKeyCode() == WXK_RETURN ||
    event.GetKeyCode() == WXK_TAB)
  {
    return !Command((char)event.GetKeyCode());
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

void wxExVi::Put(bool after)
{
  if (GetSTC()->GetReadOnly() || GetSTC()->HexMode())
  {
    return;
  }
  
  const bool lines = wxExClipboardGet().Contains("\n");
  
  if (lines)
  {
    if (after) GetSTC()->LineDown();
    GetSTC()->Home();
  }

  GetSTC()->Paste();

  if (lines && after)
  {
    GetSTC()->LineUp();
  }
}        

void wxExVi::SetIndicator(
  const wxExIndicator& indicator, 
  int start, 
  int end)
{
  if (!wxExLexers::Get()->IndicatorIsLoaded(indicator))
  {
    return;
  }

  GetSTC()->SetIndicatorCurrent(indicator.GetNo());
  GetSTC()->IndicatorFillRange(start, end - start);
}

void wxExVi::SetInsertMode(
  const wxUniChar c, 
  int repeat, 
  bool overtype,
  bool dot)
{
  if (GetSTC()->GetReadOnly() || GetSTC()->HexMode())
  {
    return;
  }
    
  if (!dot)
  {
    m_InsertMode = true;
    m_InsertText.clear();
    m_InsertRepeatCount = repeat;
    GetSTC()->BeginUndoAction();
  }

  switch ((int)c)
  {
    case 'a': GetSTC()->CharRight(); 
      break;

    case 'i': 
      break;

    case 'o': 
      GetSTC()->LineEnd(); 
      GetSTC()->NewLine(); 
      break;
    case 'A': GetSTC()->LineEnd(); 
      break;

    case 'C': 
    case 'R': 
      GetSTC()->SetSelectionStart(GetSTC()->GetCurrentPos());
      GetSTC()->SetSelectionEnd(GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine()));
      break;

    case 'I': 
      GetSTC()->Home(); 
      break;

    case 'O': 
      GetSTC()->Home(); 
      GetSTC()->NewLine(); 
      GetSTC()->LineUp(); 
      break;

    default: wxFAIL;
  }

  if (dot)
  {
    GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
    
    if (c == 'R' || c == 'C')
    {
      GetSTC()->ReplaceSelection(m_InsertText);
    }
    else
    {
      GetSTC()->AddText(m_InsertText);
    }
    
    GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos());
    GetSTC()->MarkTargetChange();
  }
  else
  {
    GetSTC()->SetOvertype(overtype);
  }
}

void wxExVi::ToggleCase()
{
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
}
#endif // wxUSE_GUI
