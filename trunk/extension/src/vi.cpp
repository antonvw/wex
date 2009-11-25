////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExSTC vi mode
// Author:    Anton van Wezenbeek
// Created:   2009-11-21
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/textdlg.h> 
#include <wx/tokenzr.h> 
#include <wx/extension/vi.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

wxExVi::wxExVi(wxExSTC* stc)
  : m_STC(stc)
  , m_InsertMode(false)
  , m_SearchText(stc->GetSearchText())
{
}

void wxExVi::Delete(
  const wxString& begin_address, 
  const wxString& end_address)
{
  if (!SetSelection(begin_address, end_address))
  {
    return;
  }

  m_STC->Cut();
}

void wxExVi::DoCommand(const wxString& command)
{
  // [address] m destination
  // [address] s [/pattern/replacement/] [options] [count]
  wxStringTokenizer tkz(command, "dmsy");
  const wxString address = tkz.GetNextToken();
  const wxChar cmd = tkz.GetLastDelimiter();
    
  wxString begin_address;
  wxString end_address;
    
  if (address == ".")
  {
    begin_address = address;
    end_address = address;
  }
  else if (address == "%")
  {
    begin_address = "1";
    end_address = "$";
  }
  else
  {
    begin_address = address.BeforeFirst(',');
     end_address = address.AfterFirst(',');
  }
      
  switch (cmd)
  {
  case 'd':
    Delete(begin_address, end_address);
    break;
  case 'm':
    Move(begin_address, end_address, tkz.GetString());
    break;
  case 's':
    {
    wxStringTokenizer tkz(tkz.GetString(), "/");

    tkz.GetNextToken(); // skip empty token
    const wxString pattern = tkz.GetNextToken();
    const wxString replacement = tkz.GetNextToken();
  
    Substitute(begin_address, end_address, pattern, replacement);
    }
    break;
  case 'y':
    Yank(begin_address, end_address);
    break;
  }
}

void wxExVi::InsertMode()
{
  m_InsertMode = true;
  m_InsertText.clear();
}

void wxExVi::LineEditor(const wxString& command)
{
  if (command.empty())
  {
    // Do nothing.
  }
  else if (command == "$")
  {
    m_STC->DocumentEnd();
  }
  else if (command == ".=")
  {
    m_STC->CallTipShow(
      m_STC->GetCurrentPos(), 
      wxString::Format("%d", m_STC->GetCurrentLine() + 1));
  }
  else if (command.IsNumber())
  {
    m_STC->GotoLine(atoi(command.c_str()) - 1);
  }
  else if (command.StartsWith("w"))
  {
    m_STC->FileSave();
  }
  else
  {
    m_LastCommand = command;
    m_InsertText.clear();
    DoCommand(command);
  }
}

void wxExVi::Move(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& destination)
{
  const int dest_line = ToLineNumber(destination);

  if (dest_line == 0)
  {
    return;
  }

  if (!SetSelection(begin_address, end_address))
  {
    return;
  }

  m_STC->BeginUndoAction();

  m_STC->Cut();
  m_STC->GotoLine(dest_line - 1);
  m_STC->Paste();

  m_STC->EndUndoAction();
}

bool wxExVi::OnKey(wxKeyEvent& event)
{
  if (m_InsertMode)
  {
    if (event.GetKeyCode() == WXK_ESCAPE)
    {
      m_InsertMode = false;
    }

    if (wxIsalnum(event.GetKeyCode()))
    {
      m_InsertText += 
        (!event.ShiftDown() ? wxTolower(event.GetUnicodeKey()): event.GetUnicodeKey());
    }
    
    return true;
  }
  
  if(
    !event.ShiftDown() &&
    !event.ControlDown())
  {
    m_Command += event.GetUnicodeKey();
  }
  
  int repeat = atoi(m_Command.c_str());

  if (repeat == 0)
  {
    repeat++;
  }
  
  bool handled_command = true;

  // Handle multichar commands.
  if (m_Command.EndsWith("CW"))
  {
    for (int i = 0; i < repeat; i++) m_STC->WordRightExtend();
    InsertMode();
  }
  else if (m_Command.EndsWith("DD"))
  {
    const int line = m_STC->LineFromPosition(m_STC->GetCurrentPos());
    const int start = m_STC->PositionFromLine(line);
    const int end = m_STC->GetLineEndPosition(line + repeat - 1);
    m_STC->SetSelectionStart(start);
    m_STC->SetSelectionEnd(end);
    m_STC->Cut();
  }
  else if (m_Command.EndsWith("DW"))
  {
    m_STC->BeginUndoAction();
    for (int i = 0; i < repeat; i++) m_STC->DelWordRight();
    m_STC->EndUndoAction();
  }
  else if (m_Command.Matches("*F?"))
  {
    for (int i = 0; i < repeat; i++) m_STC->FindNext(m_Command.Last(), wxSTC_FIND_REGEXP);
  }
  else if (m_Command.EndsWith("YY"))
  {
    const int line = m_STC->LineFromPosition(m_STC->GetCurrentPos());
    const int start = m_STC->PositionFromLine(line);
    const int end = m_STC->GetLineEndPosition(line + repeat - 1);
    m_STC->CopyRange(start, end);
  }
  else
  {
    if (!event.ShiftDown() && !event.ControlDown())
    {
      switch (event.GetKeyCode())
      {
        case '0': 
          if (m_Command.length() == 1)
          {
            m_STC->Home(); 
          }
          else
          {
            handled_command = false;
          }
          break;
        case 'A': InsertMode(); m_STC->CharRight(); break;
        case 'B': for (int i = 0; i < repeat; i++) m_STC->WordLeft(); break;
        case 'G': m_STC->DocumentStart(); break;
        case 'H': 
        case WXK_LEFT:
          for (int i = 0; i < repeat; i++) m_STC->CharLeft(); 
          break;
        case 'I': InsertMode(); break;
        case 'J': 
        case WXK_DOWN:
          for (int i = 0; i < repeat; i++) m_STC->LineDown(); 
          break;
        case 'K': 
        case WXK_UP:
          for (int i = 0; i < repeat; i++) m_STC->LineUp(); 
          break;
        case 'L': 
        case ' ': 
        case WXK_RIGHT:
          for (int i = 0; i < repeat; i++) m_STC->CharRight(); 
          break;
        case 'N': 
          for (int i = 0; i < repeat; i++) 
            m_STC->FindNext(m_SearchText, wxSTC_FIND_REGEXP);
          break;
        case 'P': 
          {
          const int pos = m_STC->GetCurrentPos();
          m_STC->LineDown();
          m_STC->Home();
          m_STC->Paste();
          m_STC->GotoPos(pos);
          }
          break;
        case 'W': for (int i = 0; i < repeat; i++) m_STC->WordRight(); break;
        case 'U': m_STC->Undo(); break;
        case 'X': m_STC->DeleteBack(); break;

        case '/': 
          {
            wxTextEntryDialog dlg(
              m_STC, 
              "/", 
              "vi",
              m_SearchText);

            if (dlg.ShowModal() == wxID_OK)
            {
              m_SearchText = dlg.GetValue();
              m_STC->FindNext(m_SearchText, wxSTC_FIND_REGEXP);
            }
          }
          break;

        // Repeat last text changing command.
        case '.': 
          if (!m_InsertText.empty())
          {
            m_STC->AddText(m_InsertText);
          }
          else
          {
            DoCommand(m_LastCommand);
          }
          break;

        case '[':
        case ']':
          {
          const int brace_match = m_STC->BraceMatch(m_STC->GetCurrentPos());
          if (brace_match != wxSTC_INVALID_POSITION)
          {
            m_STC->GotoPos(brace_match);
          }
          }
          break;
          
        case WXK_RETURN:
          m_STC->LineDown();
          break;

        default:
          handled_command = false;
      }
    }
    else if (event.ShiftDown() && !event.ControlDown())
    {
      switch (event.GetKeyCode())
      { 
        case 'A': InsertMode(); m_STC->LineEnd(); break;
        case 'D': m_STC->DelLineRight(); break;
        case 'G': 
          if (repeat > 1)
          {
            m_STC->GotoLine(repeat - 1);
          }
          else
          {
            m_STC->DocumentEnd();
          }
          break;
        case 'N': 
          for (int i = 0; i < repeat; i++) 
            m_STC->FindNext(m_SearchText, wxSTC_FIND_REGEXP, false);
          break;
        case 'P': 
          {
          m_STC->LineUp();
          m_STC->Home();
          m_STC->Paste();
          }
          break;
        // Reverse case current char.
        case '1': // TODO: Should be ~, that does not work
          {
            wxString text(m_STC->GetTextRange(
              m_STC->GetCurrentPos(), 
              m_STC->GetCurrentPos() + 1));

            wxIslower(text[0]) ? text.UpperCase(): text.LowerCase();

            m_STC->wxStyledTextCtrl::Replace(
              m_STC->GetCurrentPos(), 
              m_STC->GetCurrentPos() + 1, 
              text);

            m_STC->CharRight();
          }
          break;

        case '4': m_STC->LineEnd(); break; // $
        case '[': m_STC->ParaUp(); break; // {
        case ']': m_STC->ParaDown(); break; // }

        case ';': // :
          {
            wxTextEntryDialog dlg(m_STC, ":", "vi");

            if (dlg.ShowModal() == wxID_OK)
            {
              LineEditor(dlg.GetValue());
            }
          }
          break;

        case '/': 
          {
            wxTextEntryDialog dlg(
              m_STC, 
              "?", 
              "vi",
              m_SearchText);

            if (dlg.ShowModal() == wxID_OK)
            {
              m_SearchText = dlg.GetValue();
              m_STC->FindNext(m_SearchText, wxSTC_FIND_REGEXP, false);
            }
          }
          break;

        default:
          handled_command = false;
      }
    }
    else if (event.ControlDown())
    {
      switch (event.GetKeyCode())
      {
        case 'B': for (int i = 0; i < repeat; i++) m_STC->PageUp(); break;
        case 'E': for (int i = 0; i < repeat; i++) m_STC->LineScrollUp(); break;
        case 'F': for (int i = 0; i < repeat; i++) m_STC->PageDown(); break;
        case 'Y': for (int i = 0; i < repeat; i++) m_STC->LineScrollDown(); break;
        default:
          handled_command = false;
      }
    }
    else
    {
      wxFAIL;
    }
  }

  if (handled_command)
  {
    m_Command.clear();
  }
  
  return false;
}

bool wxExVi::SetSelection(
  const wxString& begin_address, 
  const wxString& end_address)
{
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0)
  {
    return false;
  }

  m_STC->SetSelectionStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetSelectionEnd(m_STC->GetLineEndPosition(end_line - 1));

  return true;
}

void wxExVi::Substitute(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& pattern,
  const wxString& replacement)
{
  m_STC->SetSearchFlags(wxSTC_FIND_REGEXP);

  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0)
  {
    return;
  }

  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(end_line - 1));

  while (m_STC->SearchInTarget(pattern) > 0)
  {
    const int start = m_STC->GetTargetStart();
    const int length = m_STC->ReplaceTarget(replacement);
    m_STC->SetTargetStart(start + length);
    m_STC->SetTargetEnd(m_STC->GetLineEndPosition(end_line - 1));
  }

  m_STC->EndUndoAction();
}

int wxExVi::ToLineNumber(const wxString& address) const
{
  if (address == "$")
  {
    return m_STC->GetLineCount() + 1;
  }

  wxString filtered_address(address);
  int dot = 0;

  if (filtered_address.Contains("."))
  {
    dot = m_STC->GetCurrentLine() + 1;
    filtered_address.Replace(".", "");
    if (!filtered_address.IsNumber()) return 0;
  }

  const int line_no = dot + atoi(filtered_address.c_str());
  
  if (line_no < 0)
  {
    return 0;
  }
  
  return line_no;
}

void wxExVi::Yank(
  const wxString& begin_address, 
  const wxString& end_address)
{
  if (!SetSelection(begin_address, end_address))
  {
    return;
  }

  m_STC->Copy();
}

#endif // wxUSE_GUI
