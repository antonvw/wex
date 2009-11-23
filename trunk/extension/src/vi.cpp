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

void wxExVi::LineEditor(const wxString& command)
{
  if (command == "$")
  {
    m_STC->DocumentEnd();
  }
  else if (command == ".=")
  {
    wxMessageBox(wxString::Format("%d", m_STC->GetCurrentLine() + 1));
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
    // [address] m destination
    // [address] s [/pattern/replacement/] [options] [count]
    const wxString begin_address = command.BeforeFirst(',');
    wxString rest = command.AfterFirst(',');
    wxStringTokenizer tkz(rest, "mb");
    const wxString end_address = tkz.GetNextToken();
    const wxChar cmd = tkz.GetLastDelimiter();

    rest = tkz.GetString();

    switch (cmd)
    {
    case 'c':
      {
      wxStringTokenizer tkz(rest, "/");

      const wxString pattern = tkz.GetNextToken();
      const wxString replacement = tkz.GetNextToken();

      Substitute(begin_address, end_address, pattern, replacement);
      }
      break;
    case 'd':
      Delete(begin_address, end_address);
      break;
    case 'm':
      Move(begin_address, end_address, rest);
      break;
    case 'y':
      Yank(begin_address, end_address);
      break;
    }
  }
}

void wxExVi::Move(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& destination)
{
  if (!SetSelection(begin_address, end_address))
  {
    return;
  }

  m_STC->Cut();

  if (destination.IsNumber())
  {
    int dest_line = atoi(destination.c_str());
    m_STC->GotoLine(dest_line - 1);
  }

  m_STC->Paste();
}

void wxExVi::OnKey(wxKeyEvent& event)
{
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
    m_InsertMode = true;
  }
  else if (m_Command.EndsWith("DD"))
  {
    for (int i = 0; i < repeat; i++) m_STC->LineDelete();
  }
  else if (m_Command.EndsWith("DW"))
  {
    for (int i = 0; i < repeat; i++) m_STC->DelWordRight();
  }
  else if (m_Command.Matches("*F?"))
  {
    for (int i = 0; i < repeat; i++) m_STC->FindNext(m_Command.Last());
  }
  else if (m_Command.EndsWith("YY"))
  {
    const int line = m_STC->LineFromPosition(m_STC->GetCurrentPos());
    const int start = m_STC->PositionFromLine(line);
    const int end = m_STC->GetLineEndPosition(line + repeat);
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
        case 'A': m_InsertMode = true; m_STC->CharRight(); break;
        case 'B': for (int i = 0; i < repeat; i++) m_STC->WordLeft(); break;
        case 'G': m_STC->DocumentStart(); break;
        case 'H': for (int i = 0; i < repeat; i++) m_STC->CharLeft(); break;
        case 'I': m_InsertMode = true; break;
        case 'J': for (int i = 0; i < repeat; i++) m_STC->LineDown(); break;
        case 'K': for (int i = 0; i < repeat; i++) m_STC->LineUp(); break;
        case 'L': for (int i = 0; i < repeat; i++) m_STC->CharRight(); break;
        case 'N': 
          for (int i = 0; i < repeat; i++) 
            m_STC->FindNext(m_STC->GetSearchText());
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
              m_STC->GetSearchText());

            if (dlg.ShowModal() == wxID_OK)
            {
              m_STC->FindNext(dlg.GetValue());
            }
          }
          break;

        // Repeat last text changing command.
        case '.': 
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

        default:
          handled_command = false;
      }
    }
    else if (event.ShiftDown() && !event.ControlDown())
    {
      switch (event.GetKeyCode())
      { 
        case 'A': m_InsertMode = true; m_STC->LineEnd(); break;
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
            m_STC->FindNext(m_STC->GetSearchText(), false);
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
              m_STC->GetSearchText());

            if (dlg.ShowModal() == wxID_OK)
            {
              m_STC->FindNext(dlg.GetValue(), false);
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
}

bool wxExVi::SetSelection(
  const wxString& begin_address, 
  const wxString& end_address)
{
  const int begin_line = atoi(begin_address.c_str());
  const int end_line = 
    (end_address == "." ? m_STC->GetLineCount(): atoi(end_address.c_str()));

  if (begin_line == 0 || end_line == 0)
  {
    return false;
  }

  m_STC->SetSelectionStart(m_STC->PositionFromLine(begin_line));
  m_STC->SetSelectionEnd(m_STC->PositionFromLine(end_line));

  return true;
}

void wxExVi::Substitute(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& pattern,
  const wxString& replacement)
{
  if (!SetSelection(begin_address, end_address))
  {
    return;
  }

  m_STC->ReplaceAll(pattern, replacement, true);
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
