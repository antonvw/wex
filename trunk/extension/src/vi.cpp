/******************************************************************************\
* File:          stc.cpp
* Purpose:       Implementation of class wxExSTC
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: stc.cpp 2228 2009-11-21 09:00:23Z antonvw $
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/stc.h>

#if wxUSE_GUI

void wxExSTC::OnKeyVi(wxKeyEvent& event)
{
  if(
    !event.ShiftDown() &&
    !event.ControlDown())
  {
    m_viCommand += event.GetUnicodeKey();
  }
  
  int repeat = atoi(m_viCommand.c_str());

  if (repeat == 0)
  {
    repeat++;
  }
  
  bool handled_command = true;

  if (m_viCommand.EndsWith("CW"))
  {
    for (int i = 0; i < repeat; i++) WordRightExtend();
    m_viInsertMode = true;
  }
  else if (m_viCommand.EndsWith("DD"))
  {
    for (int i = 0; i < repeat; i++) LineDelete();
  }
  else if (m_viCommand.EndsWith("DW"))
  {
    for (int i = 0; i < repeat; i++) DelWordRight();
  }
  else if (m_viCommand.EndsWith("YY"))
  {
    const int line = LineFromPosition(GetCurrentPos());
    const int start = PositionFromLine(line);
    const int end = GetLineEndPosition(line + repeat);
    CopyRange(start, end);
  }
  else
  {
    if (!event.ShiftDown() && !event.ControlDown())
    {
      switch (event.GetKeyCode())
      {
        case '0': 
          if (m_viCommand.length() == 1)
          {
            Home(); 
          }
          else
          {
            handled_command = false;
          }
          break;
        case 'A': m_viInsertMode = true; CharRight(); break;
        case 'B': for (int i = 0; i < repeat; i++) WordLeft(); break;
        case 'G': DocumentStart(); break;
        case 'H': for (int i = 0; i < repeat; i++) CharLeft(); break;
        case 'I': m_viInsertMode = true; break;
        case 'J': for (int i = 0; i < repeat; i++) LineDown(); break;
        case 'K': for (int i = 0; i < repeat; i++) LineUp(); break;
        case 'L': for (int i = 0; i < repeat; i++) CharRight(); break;
        case 'N': 
          for (int i = 0; i < repeat; i++) 
            FindNext(wxExFindReplaceData::Get()->GetFindString());
        case 'P': 
          {
          const int pos = GetCurrentPos();
          LineDown();
          Home();
          Paste();
          GotoPos(pos);
          }
          break;
        case 'W': for (int i = 0; i < repeat; i++) WordRight(); break;
        case 'U': Undo(); break;
        case 'X': DeleteBack(); break;

        case '/': 
          {
          wxASSERT(wxTheApp != NULL);
          wxWindow* window = wxTheApp->GetTopWindow();
          wxASSERT(window != NULL);
          wxFrame* frame = wxDynamicCast(window, wxFrame);
          wxASSERT(frame != NULL);
          wxPostEvent(frame, 
            wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_FIND));
          }
          break;

        // Repeat last text changing command.
        case '.': 
          break;

        case '[':
        case ']':
          {
          const int brace_match = BraceMatch(GetCurrentPos());
          if (brace_match != wxSTC_INVALID_POSITION)
          {
            GotoPos(brace_match);
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
        case 'A': m_viInsertMode = true; LineEnd(); break;
        case 'D': DelLineRight(); break;
        case 'G': 
          if (repeat > 1)
          {
            GotoLine(repeat - 1);
          }
          else
          {
            DocumentEnd();
          }
          break;
        case 'N': 
          for (int i = 0; i < repeat; i++) 
            FindNext(wxExFindReplaceData::Get()->GetFindString(), false);
          break;
        case 'P': 
          {
          LineUp();
          Home();
          Paste();
          }
          break;
        // Reverse case current char.
        case '1': // TODO: Should be ~, that does not work
          {
            wxString text(GetTextRange(GetCurrentPos(), GetCurrentPos() + 1));
            wxIslower(text[0]) ? text.UpperCase(): text.LowerCase();
            wxStyledTextCtrl::Replace(GetCurrentPos(), GetCurrentPos() + 1, text);
            CharRight();
          }
          break;

        case '4': LineEnd(); break; // $
        case '[': ParaUp(); break; // {
        case ']': ParaDown(); break; // }

        default:
          handled_command = false;
      }
    }
    else if (event.ControlDown())
    {
      switch (event.GetKeyCode())
      {
        case 'B': for (int i = 0; i < repeat; i++) PageUp(); break;
        case 'E': for (int i = 0; i < repeat; i++) LineScrollUp(); break;
        case 'F': for (int i = 0; i < repeat; i++) PageDown(); break;
        case 'Y': for (int i = 0; i < repeat; i++) LineScrollDown(); break;
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
    m_viCommand.clear();
  }
}  

#endif // wxUSE_GUI
