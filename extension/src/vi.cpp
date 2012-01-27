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

wxString wxExVi::m_LastCommand;
wxString wxExVi::m_LastFindCharCommand;

wxExVi::wxExVi(wxExSTC* stc)
  : wxExEx(stc)
  , m_Dot(false)
  , m_MarkerSymbol(0, -1)
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
  
  if (!m_InsertMode)
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
        return GetSTC()->FindNext(
          command.Mid(1),
          GetSearchFlags(),
          command.StartsWith("/"));
      }
      else
      {
        GetFrame()->GetExCommand(this, command);
        return true;
      }
    }
  }
  
  bool handled = true;

  switch ((int)command.Last())
  {
    case WXK_CONTROL_E: ChangeNumber(true); break;
    case WXK_CONTROL_J: ChangeNumber(false); break;
      
    case WXK_BACK:
      if (m_InsertMode)
      {
        if (m_InsertText.size() > 1)
        {
          m_InsertText.Truncate(m_InsertText.size() - 1);
        }
        
        if (MacroIsPlayback())
        {
          GetSTC()->CharLeft();
        }
      }
      else
      {
        GetSTC()->CharLeft();
      }
      break;
      
    case WXK_ESCAPE:
      if (m_InsertMode)
      {
        // Add extra inserts if necessary.        
        for (int i = 1; i < m_InsertRepeatCount; i++)
        {
          GetSTC()->AddText(m_InsertText);
        }
        
        GetSTC()->EndUndoAction();
        
        m_InsertMode = false;
      }
      else
      {
        wxBell();
      }

      if (!GetSTC()->GetSelectedText().empty())
      {
        GetSTC()->SetSelection(GetSTC()->GetCurrentPos(), GetSTC()->GetCurrentPos());
      }
      break;

    case WXK_RETURN:
      if (!m_InsertMode)
      {
        int repeat = atoi(m_Command.c_str());

        if (repeat == 0)
        {
          repeat++;
        }
  
        for (int i = 0; i < repeat; i++) GetSTC()->LineDown();

        m_LastCommand = m_Command + GetSTC()->GetEOL();
      }
      else
      {
        m_InsertText += command.Last();
        
        if (MacroIsPlayback())
        {
          GetSTC()->NewLine();
        }
      }
      break;
    
    case WXK_TAB:
      if (m_Command.size() > 0 && wxRegEx("[0-9]*r").Matches(m_Command))
      {
        int repeat = atoi(m_Command.c_str());

        if (repeat == 0)
        {
          repeat++;
        }
  
        GetSTC()->SetTargetStart(GetSTC()->GetCurrentPos());
        GetSTC()->SetTargetEnd(GetSTC()->GetCurrentPos() + repeat);
        GetSTC()->ReplaceTarget(wxString('\t', repeat));
        GetSTC()->MarkTargetChange();
          
        m_LastCommand = m_Command + "\t";
      }
      
      if (m_InsertMode && MacroIsPlayback())
      {
        GetSTC()->AddText("\t");
      }
      break;
      
    default: handled = false;
  }
  
  if (handled)
  {  
    MacroRecord(command);
    m_Command.clear();
    
    if (!MacroIsPlayback())
    {
      return !m_InsertMode;
    }
    else
    {
      return true;
    }
  }
  
  if (m_InsertMode)
  {
    GetSTC()->AddText(command);
    GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
    return true;
  }
  
  handled = true;
  
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
      InsertMode();
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
      InsertMode();
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
    DeleteMarker(command.Last());
    m_Markers[command.Last()] = GetSTC()->GetCurrentLine() + 1;
    GetSTC()->MarkerAdd(GetSTC()->GetCurrentLine(), m_MarkerSymbol.GetNo());
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
#ifdef wxExUSE_CPP0X	
    const auto it = m_Markers.find(command.Last());
#else
    const std::map<wxUniChar, int>::iterator it = m_Markers.find(command.Last());
#endif	

    if (it != m_Markers.end())
    {
      GetSTC()->GotoLineAndSelect(it->second);
    }
    else
    {
      wxBell();
    }
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
        InsertMode(command.Last(), repeat, false, m_Dot); 
        break;
      case 'R': 
        InsertMode(command.Last(), repeat, true, m_Dot); 
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
      case 'P': Put(false); break;
      
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
      
      case 'w': for (int i = 0; i < repeat; i++) GetSTC()->WordRight(); break;
      case 'u': GetSTC()->Undo(); break;
      case 'x': 
        if (GetSTC()->HexMode()) return false;
        for (int i = 0; i < repeat; i++) 
        {
          GetSTC()->CharRight();
          GetSTC()->DeleteBack(); 
        }
        GetSTC()->MarkerAddChange(GetSTC()->GetCurrentLine());
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
      case 'M': GetSTC()->GotoLine(
        GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen() / 2);
        break;
      case 'L': GetSTC()->GotoLine(
        GetSTC()->GetFirstVisibleLine() + GetSTC()->LinesOnScreen()); 
        break;
      case 'N': 
        for (int i = 0; i < repeat; i++) 
          if (!GetSTC()->FindNext(
            wxExFindReplaceData::Get()->GetFindString(), 
            GetSearchFlags(), 
            !m_SearchForward)) break;
        break;
      case 'X': 
        if (GetSTC()->HexMode()) return false;
        for (int i = 0; i < repeat; i++) GetSTC()->DeleteBack(); break;

      case '.': 
        m_Dot = true;
        Command(m_LastCommand); 
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
      
      case 2:  // ^b
        for (int i = 0; i < repeat; i++) GetSTC()->PageUp(); 
        break;
      case 7:  // ^g (^f is not possible, already find accel key)
        for (int i = 0; i < repeat; i++) GetSTC()->PageDown(); 
        break;
      case 16: // ^p (^y is not possible, already redo accel key)
        for (int i = 0; i < repeat; i++) GetSTC()->LineScrollUp(); 
        break;
      case 17: // ^q (^n is not possible, already new doc accel key)
        for (int i = 0; i < repeat; i++) GetSTC()->LineScrollDown(); 
        break;

      default:
        handled = false;
    }
  }

  if (handled)
  {  
    MacroRecord(command);
  }
  
  return handled;
}

void wxExVi::DeleteMarker(const wxUniChar& marker)
{
#ifdef wxExUSE_CPP0X	
  const auto it = m_Markers.find(marker);
#else
  const std::map<wxUniChar, int>::iterator it = m_Markers.find(marker);
#endif

  if (it != m_Markers.end())
  {
    GetSTC()->MarkerDelete(it->second - 1, m_MarkerSymbol.GetNo());
    m_Markers.erase(it);
  }
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

void wxExVi::InsertMode(
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
        if ((m_Command.length() > 1 && !m_Command.Matches("m?")) || 
          m_Command == "a" || 
          m_Command == "i" || 
          m_Command == "o" || 
          m_Command == "p" ||
          m_Command == "x" ||
          m_Command == "A" || 
          m_Command == "C" || 
          m_Command == "D" || 
          m_Command == "I" || 
          m_Command == "O" || 
          m_Command == "R" || 
          m_Command == "X" || 
          m_Command == "~"
          )
        {
          m_LastCommand = m_Command;
        }
        
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
    event.GetKeyCode() == WXK_BACK ||
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
