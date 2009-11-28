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
#include <wx/extension/frame.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

wxExVi::wxExVi(wxExSTC* stc)
  : m_STC(stc)
  , m_InsertMode(false)
  , m_SearchForward(true)
  , m_SearchText(stc->GetSearchText())
{
}

void wxExVi::Delete(int lines) const
{
  if (m_STC->GetReadOnly())
  {
    return;
  }

  const int line = m_STC->LineFromPosition(m_STC->GetCurrentPos());
  const int start = m_STC->PositionFromLine(line);
  const int end = m_STC->PositionFromLine(line + lines);

  m_STC->SetSelectionStart(start);
  m_STC->SetSelectionEnd(end);
  m_STC->Cut();

  if (lines > 2)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(wxString::Format("%d fewer lines", lines));
#endif
  }
}

void wxExVi::Delete(
  const wxString& begin_address, 
  const wxString& end_address) const
{
  if (m_STC->GetReadOnly())
  {
    return;
  }

  if (!SetSelection(begin_address, end_address))
  {
    return;
  }

  const int lines = wxExGetNumberOfLines(m_STC->GetSelectedText());
  
  m_STC->Cut();

  if (lines > 2)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(wxString::Format("%d fewer lines", lines));
#endif
  }
}


bool wxExVi::DoCommand(const wxString& command)
{
  if (command.StartsWith(":") && command.length() > 1)
  {
    return DoCommandRange(command);
  }
          
  int repeat = atoi(command.c_str());

  if (repeat == 0)
  {
    repeat++;
  }
  
  bool handled = true;

  // Handle multichar commands.
  if (command.EndsWith("cw"))
  {
    for (int i = 0; i < repeat; i++) m_STC->WordRightExtend();
    InsertMode();
  }
  else if (command == "cc")
  {
    m_STC->Home();
    m_STC->DelLineRight();
    InsertMode();
  }
  else if (command.EndsWith("dd"))
  {
    Delete(repeat);
  }
  else if (command == "d0")
  {
    m_STC->DelLineLeft();
  }
  else if (command == "d$")
  {
    m_STC->DelLineRight();
  }
  else if (command.EndsWith("dw"))
  {
    m_STC->BeginUndoAction();
    for (int i = 0; i < repeat; i++) m_STC->DelWordRight();
    m_STC->EndUndoAction();
  }
  else if (command.Matches("*f?"))
  {
    for (int i = 0; i < repeat; i++) m_STC->FindNext(command.Last(), wxSTC_FIND_REGEXP);
  }
  else if (command.Matches("*F?"))
  {
    for (int i = 0; i < repeat; i++) m_STC->FindNext(command.Last(), wxSTC_FIND_REGEXP, false);
  }
  else if (command.Matches("*J?"))
  {
    m_STC->BeginUndoAction();
    m_STC->SetTargetStart(m_STC->PositionFromLine(m_STC->GetCurrentLine()));
    m_STC->SetTargetEnd(m_STC->PositionFromLine(m_STC->GetCurrentLine() + repeat));
    m_STC->LinesJoin();
    m_STC->EndUndoAction();
 }
  else if (command.Matches("m?"))
  {
    m_Markers[command.Last()] = m_STC->GetCurrentLine();
  }
  else if (command.Matches("r?"))
  {
    m_STC->wxStyledTextCtrl::Replace(
      m_STC->GetCurrentPos(), 
      m_STC->GetCurrentPos() + 1, 
      command.Last());
  }
  else if (command.EndsWith("yw"))
  {
    const int start = m_STC->GetCurrentPos();
    for (int i = 0; i < repeat; i++) 
      m_STC->WordRight();
    m_STC->CopyRange(start, m_STC->GetCurrentPos());
    m_STC->GotoPos(start);
  }
  // This is a sepcial one, not really vi.
  else if (command.EndsWith("Yw"))
  {
    const int start = m_STC->GetCurrentPos();
    for (int i = 0; i < repeat; i++) 
      m_STC->WordRight();
    m_STC->CopyRange(start, m_STC->GetCurrentPos());
    m_SearchText = m_STC->GetTextRange(start, m_STC->GetCurrentPos()).Trim();
  }
  else if (command.EndsWith("yy"))
  {
    Yank(repeat);
  }
  else if (command == "ZZ")
  {
    if (m_STC->GetContentsChanged())
    {
      m_STC->FileSave();
    }

    wxCloseEvent event(wxEVT_CLOSE_WINDOW);
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command.Matches("'?"))
  {
    std::map<wxUniChar, int>::const_iterator it = m_Markers.find(command.Last());

    if (it != m_Markers.end())
  	{
	    m_STC->GotoLine(it->second);
	  }
  }
  else
  {
    switch ((int)command.Last())
    {
      case '0': 
        if (command.length() == 1)
        {
          m_STC->Home(); 
        }
        else
        {
          handled = false;
        }
        break;
      case 'a': InsertMode(); m_STC->CharRight(); break;
      case 'b': for (int i = 0; i < repeat; i++) m_STC->WordLeft(); break;
      case 'e': for (int i = 0; i < repeat; i++) m_STC->WordRightEnd(); break;
      case 'g': m_STC->DocumentStart(); break;
      case 'h': 
      case WXK_LEFT:
        for (int i = 0; i < repeat; i++) m_STC->CharLeft(); 
        break;
      case 'i': InsertMode(); break;
      case 'j': 
      case WXK_DOWN:
        for (int i = 0; i < repeat; i++) m_STC->LineDown(); 
        break;
      case 'k': 
      case WXK_UP:
        for (int i = 0; i < repeat; i++) m_STC->LineUp(); 
        break;
      case 'l': 
      case ' ': 
      case WXK_RIGHT:
        for (int i = 0; i < repeat; i++) m_STC->CharRight(); 
        break;
      case 'n': 
        for (int i = 0; i < repeat; i++) 
          m_STC->FindNext(m_SearchText, wxSTC_FIND_REGEXP, m_SearchForward);
        break;
      case 'o': 
        m_STC->LineEnd(); 
        m_STC->NewLine(); 
        InsertMode(); 
        break;
      case 'p': m_STC->Paste();
        break;
      case 'w': for (int i = 0; i < repeat; i++) m_STC->WordRight(); break;
      case 'u': m_STC->Undo(); break;
      case 'x': 
        {
        wxKeyEvent event(wxEVT_KEY_DOWN);
        event.SetId(WXK_DELETE);
        wxPostEvent(m_STC, event);
        }
        break;

      case 'A': InsertMode(); m_STC->LineEnd(); break;
      case 'C': 
        InsertMode(); 
        m_STC->SetSelectionStart(m_STC->GetCurrentPos());
        m_STC->SetSelectionEnd(m_STC->GetLineEndPosition(m_STC->GetCurrentLine()));
        break;
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
      case 'H': 
          m_STC->GotoLine(m_STC->GetFirstVisibleLine());
        break;
      case 'I': 
        m_STC->Home(); 
        InsertMode(); 
        break;
      case 'M': 
          m_STC->GotoLine(m_STC->GetFirstVisibleLine() + m_STC->LinesOnScreen() / 2);
        break;
      case 'L': 
          m_STC->GotoLine(m_STC->GetFirstVisibleLine() + m_STC->LinesOnScreen());
        break;
      case 'N': 
        for (int i = 0; i < repeat; i++) 
          m_STC->FindNext(m_SearchText, wxSTC_FIND_REGEXP, !m_SearchForward);
        break;
      case 'O': 
        m_STC->Home(); 
        m_STC->NewLine(); 
        m_STC->LineUp(); 
        InsertMode(); 
        break;
      case 'P':
        m_STC->GotoPos(m_STC->GetCurrentPos() - wxExClipboardGet().length());
        m_STC->Paste();
        break;
      case 'R': InsertMode(true); break;
      case 'X': m_STC->DeleteBack(); break;

      case '/': 
      case '?': 
        {
          wxTextEntryDialog dlg(
            m_STC, 
            command.Last(), 
            "vi",
            m_SearchText);

          if (dlg.ShowModal() == wxID_OK)
          {
            m_SearchForward = command.Last() == '/';
            m_SearchText = dlg.GetValue();
            m_STC->FindNext(m_SearchText, wxSTC_FIND_REGEXP, m_SearchForward);
          }
        }
        break;

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

      case '~':
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

      case '$': m_STC->LineEnd(); break;
      case '{': m_STC->ParaUp(); break;
      case '}': m_STC->ParaDown(); break;
      
      case '%':
        {
          int brace_match = m_STC->BraceMatch(m_STC->GetCurrentPos());
          
          if (brace_match != wxSTC_INVALID_POSITION)
          {
            m_STC->GotoPos(brace_match);
          }
          else
          {
            brace_match = m_STC->BraceMatch(m_STC->GetCurrentPos() - 1);
            
            if (brace_match != wxSTC_INVALID_POSITION)
            {
              m_STC->GotoPos(brace_match);
            }
          }
        }
        break;

      case ':': DoCommandLine(); break;

      case 2:  // ^b
        for (int i = 0; i < repeat; i++) m_STC->PageUp(); 
        break;
      case 5:  // ^e
        for (int i = 0; i < repeat; i++) m_STC->LineScrollUp(); 
        break;
      case 6:  // ^f
        for (int i = 0; i < repeat; i++) m_STC->PageDown(); 
        break;
      case 25: // ^y
        for (int i = 0; i < repeat; i++) m_STC->LineScrollDown(); 
        break;

      default:
        handled = false;
    }
  }

  return handled;
}

void wxExVi::DoCommandLine()
{
  wxTextEntryDialog dlg(m_STC, ":", "vi");

  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return;
  }

  const wxString command = ":" + dlg.GetValue();

  if (command == ":$")
  {
    m_STC->DocumentEnd();
  }
  else if (command == ":d")
  {
    Delete(1);
  }
  else if (command == ":y")
  {
    Yank(1);
  }
  else if (command.Last() == '=')
  {
    m_STC->CallTipShow(
      m_STC->GetCurrentPos(), 
      wxString::Format("%s%d",
        command.AfterFirst(':').c_str(), 
        ToLineNumber(command.AfterFirst(':').BeforeLast('='))));
  }
  else if (command.AfterFirst(':').IsNumber())
  {
    m_STC->GotoLine(atoi(command.AfterFirst(':').c_str()) - 1);
  }
  else if (command == ":w")
  {
    m_STC->FileSave();
  }
  else if (command == ":x")
  {
    if (m_STC->GetContentsChanged())
    {
      m_STC->FileSave();
    }

    wxCloseEvent event(wxEVT_CLOSE_WINDOW);
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command == ":q")
  {
    wxCloseEvent event(wxEVT_CLOSE_WINDOW);
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command == ":q!")
  {
    wxCloseEvent event(wxEVT_CLOSE_WINDOW);
    event.SetCanVeto(false); 
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else
  {
    if (DoCommandRange(command))
    {
      m_LastCommand = command;
      m_InsertText.clear();
    }
  }
}

bool wxExVi::DoCommandRange(const wxString& command) const
{
  // :[address] m destination
  // :[address] s [/pattern/replacement/] [options] [count]
  wxStringTokenizer tkz(command.AfterFirst(':'), "dmsy");

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

  bool handled = true;
      
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
  default:
    handled = false;
  }

  return handled;
}

void wxExVi::InsertMode(bool overtype)
{
  if (!m_STC->GetReadOnly())
  {
    m_InsertMode = true;
    m_InsertText.clear();

    m_STC->SetOvertype(overtype);
    m_STC->BeginUndoAction();
  }
}

void wxExVi::Move(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& destination) const
{
  if (m_STC->GetReadOnly())
  {
    return;
  }

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
  
  const int lines = wxExGetNumberOfLines(m_STC->GetSelectedText());
  
  if (lines > 2)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(wxString::Format("%d lines moved", lines));
#endif
  }
}

bool wxExVi::OnChar(wxKeyEvent& event)
{
  if (m_InsertMode)
  {
    m_InsertText += event.GetUnicodeKey();

    return true;
  }
  else
  {
    m_Command += event.GetUnicodeKey();

    if (DoCommand(m_Command))
    {
      // Prevent motion command and the .dot command
      // to be stored as last command.
      if (m_Command.length() > 1 || m_Command == "p")
      {
        m_LastCommand = m_Command;
      }

      m_Command.clear();
    }

    return false;
  }
}

bool wxExVi::OnKeyDown(wxKeyEvent& event)
{
  if (event.GetKeyCode() == WXK_ESCAPE)
  {
    if (m_InsertMode)
    {
      m_STC->EndUndoAction();
      m_InsertMode = false;
    }
  }

  return true;
}

bool wxExVi::SetSelection(
  const wxString& begin_address, 
  const wxString& end_address) const
{
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0)
  {
    return false;
  }

  m_STC->SetSelectionStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetSelectionEnd(m_STC->PositionFromLine(end_line));

  return true;
}

void wxExVi::Substitute(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& pattern,
  const wxString& replacement) const
{
  if (m_STC->GetReadOnly())
  {
    return;
  }

  m_STC->SetSearchFlags(wxSTC_FIND_REGEXP);

  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0)
  {
    return;
  }

  int nr_replacements = 0;

  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetTargetEnd(m_STC->PositionFromLine(end_line));

  while (m_STC->SearchInTarget(pattern) > 0)
  {
    const int start = m_STC->GetTargetStart();
    const int length = m_STC->ReplaceTarget(replacement);
    m_STC->SetTargetStart(start + length);
    m_STC->SetTargetEnd(m_STC->PositionFromLine(end_line));
    nr_replacements++;
  }

  m_STC->EndUndoAction();

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(wxString::Format(_("Replaced: %d occurrences of: %s"),
    nr_replacements, pattern.c_str()));
#endif
}

int wxExVi::ToLineNumber(const wxString& address) const
{
  // Check if we are referring to a defined marker.
  if (address.StartsWith("'"))
  {
    std::map<wxUniChar, int>::const_iterator it = m_Markers.find(address.Last());

    if (it != m_Markers.end())
	  {
	    return it->second + 1;
	  }
	  else
    {
      return 0;
    }
  }

  // Calculate and filter out a dot or a dollar.
  wxString filtered_address(address);
  int dot = 0;

  if (filtered_address.Contains("."))
  {
    dot = m_STC->GetCurrentLine() + 1;
    filtered_address.Replace(".", "");
  }

  int dollar = 0;

  if (filtered_address.Contains("$"))
  {
    dollar = m_STC->GetLineCount();
    filtered_address.Replace("$", "");
  }

  if (!filtered_address.IsNumber()) return 0;

  // Calculate the line.
  const int line_no = dot + dollar + atoi(filtered_address.c_str());
  
  // Limit the range of what is returned.
  if (line_no < 0)
  {
    return 1;
  }
  else if (line_no > m_STC->GetLineCount())
  {
    return m_STC->GetLineCount();
  }  
  else
  {
    return line_no;
  }
}

void wxExVi::Yank(int lines) const
{
  const int line = m_STC->LineFromPosition(m_STC->GetCurrentPos());
  const int start = m_STC->PositionFromLine(line);
  const int end = m_STC->PositionFromLine(line + lines);

  m_STC->CopyRange(start, end);

  if (lines > 2)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(wxString::Format("%d lines yanked", lines));
#endif
  }
}

void wxExVi::Yank(
  const wxString& begin_address, 
  const wxString& end_address) const
{
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0)
  {
    return;
  }

  const int start = m_STC->PositionFromLine(begin_line);
  const int end = m_STC->PositionFromLine(end_line);

  m_STC->CopyRange(start, end);
  
  const int lines = end_line - begin_line;

  if (lines > 2)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(wxString::Format("%d lines yanked", lines));
#endif
  }
}

#endif // wxUSE_GUI
