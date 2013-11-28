////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
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
wxString wxExVi::m_LastFindCharCommand;

wxExVi::wxExVi(wxExSTC* stc)
  : wxExEx(stc)
  , m_Dot(false)
  , m_Mode(MODE_NORMAL)
  , m_InsertRepeatCount(1)
  , m_SearchForward(true)
{
}

void wxExVi::AddText(const wxString& text)
{
  if (!GetSTC()->GetOvertype())
  {
    GetSTC()->AddText(text);
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

bool wxExVi::Command(const wxString& command)
{
  if (command.empty())
  {
    return false;
  }
  
  if (
      command.StartsWith("=") ||
     (GetMacros().IsPlayback() &&
      command.StartsWith(wxUniChar(WXK_CONTROL_R) + wxString("="))))
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
      if (command.length() > 1)
      {
        m_SearchForward = command.StartsWith("/");
          
        // This is a previous entered command.
        handled = GetSTC()->FindNext(
          command.Mid(1),
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
      if (!m_Dot && command.Last() == WXK_ESCAPE)
      {
        if (!GetSTC()->GetSelectedText().empty())
        {
          GetSTC()->SelectNone();
        }
        
        if (m_Mode == MODE_NORMAL)
        {
          wxBell();
          
          m_Command.clear();

          if (GetMacros().IsRecording())
          {
            GetMacros().StopRecording();
          }
        }
        else
        {
          m_Mode = MODE_NORMAL;
        }
      }
      // Handle multichar commands.
      else
      {
        wxString rest(command);
        long int repeat = 1;
        
        if (rest.StartsWith("\""))
        {
          if (rest.size() < 2)
          {
            return false;
          }
          
          SetRegister(rest.Mid(1, 1));
          rest = rest.Mid(2);
        }
        else
        {
          SetRegister(wxEmptyString);

          int seq_size = 0; // size of sequence of digits from begin in rest
          
          for (int i = 0; i < rest.size(); i++)
          {
            if (rest[i] > 255 || rest[i] < 0 || !isdigit(rest[i]))
              break;
            seq_size++;
          }
          
          if (seq_size > 0)
          {
            repeat = strtol(rest.Mid(0, seq_size).c_str(), NULL, 10);
            rest = rest.Mid(seq_size);
          }
        }
  
        switch (rest.size())
        {
          case 0: return false;
          case 1: handled = CommandChar((int)rest.GetChar(0), repeat); break;
          
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
              
                InsertMode(rest.Mid(2));
              }
              return true;
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
              
                InsertMode(rest.Mid(2));
              }
              return true;
            case CHR_TO_NUM('d','d'): wxExAddressRange(this, repeat).Delete(); break;
            case CHR_TO_NUM('d','e'):
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
              break;
            case CHR_TO_NUM('d','w'):
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
              break;
            case CHR_TO_NUM('d','0'):
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
              break;
            case CHR_TO_NUM('d','$'):
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
              break;
            case CHR_TO_NUM('y','w'):
              for (int i = 0; i < repeat; i++) GetSTC()->WordRightEnd();
              for (int j = 0; j < repeat; j++) GetSTC()->WordLeftExtend();
                
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
              if (CommandChar((int)rest.GetChar(0), repeat))
              {
                // do nothing
              }
              else if (rest.Matches("f?") || rest.Matches("F?"))
              {
                for (int i = 0; i < repeat; i++) 
                  GetSTC()->FindNext(rest.Last(), GetSearchFlags(), rest[0] == 'f');
                m_LastFindCharCommand = command;
              }
              else if (OneLetterAfter("m", rest))
              {
                MarkerAdd(rest.Last());
              }
              else if (OneLetterAfter("q", rest))
              {
                if (!GetMacros().IsRecording())
                {
                  MacroStartRecording(rest.Mid(1));
                  return true; // as we should not do default actions
                }
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
              else if (rest.Matches("t?") || rest.Matches("T?"))
              {
                for (int i = 0; i < repeat; i++) 
                  GetSTC()->FindNext(rest.Last(), GetSearchFlags(), rest[0] == 't');
                GetSTC()->CharLeft();
                m_LastFindCharCommand = command;
              }
              else if (OneLetterAfter("'", rest))
              {
                MarkerGoto(rest.Last());
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
                return true;
              }  
              else if (rest.StartsWith("@"))
              {
                std::vector <wxString> v;
                  
                if (wxExMatch("@(.+)@", rest, v) > 0)
                {
                  handled = MacroPlayback(v[0], repeat);
                  
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
                
                auto v(GetMacros().GetRegisters());
                  
                for (auto it = v.begin();  it != v.end(); ++it)
                {
                  output += *it + "\n";
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
              else
              {
                handled = false;
              }
            } // switch (CHR_TO_NUM((int)rest[0], (int)rest[1]))
        } // switch (rest.size())
      
        if (handled && m_Mode == MODE_INSERT)
        {
          InsertMode(rest.Mid(1));
          return true;
        }
      } // Handle multichar commands.
  } // switch (command[0])

  if (!handled)
  {  
    if (m_Mode == MODE_VISUAL || m_Mode == MODE_VISUAL_LINE)
    {
      if (!command.Contains("'<,'>"))
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
  const wxString calc = command.Mid(index);
  
  wxStringTokenizer tkz(calc, "+-*/");

  double sum = 0;
  bool init = true;
  wxChar prev_cmd = 0;
  int width = 0;

  while (tkz.HasMoreTokens())
  {
    wxString token = tkz.GetNextToken();
    token.Trim(true);
    token.Trim(false);
    
    const int new_width = token.AfterFirst(',').length();
    if (new_width > width) width = new_width;
    
    double value;
  
    if (token.StartsWith(wxUniChar(WXK_CONTROL_R)))
    {
      const wxChar c = token[1];
    
      switch (c)
      {
      case '\"':
        value = atof(wxExClipboardGet()); break;
          
      default:
        value = atof(GetMacros().GetRegister(c));
      }
    }
    else
    {
      value = atof(token);
    }
    
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
  
  if (m_Mode == MODE_INSERT)
  {
    if (GetLastCommand().EndsWith("cw"))
    {
      GetSTC()->ReplaceSelection(wxEmptyString);
    }
  
    AddText(wxString::Format("%.*f", width, sum));
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
        
    case 'b': 
      VisualExtendLeftLine();
      for (int i = 0; i < repeat; i++)
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->WordLeft(); break;
          case MODE_VISUAL: GetSTC()->WordLeftExtend(); break;
        }
      }
      break;

    case 'e': 
      VisualExtendRightLine();
      for (int i = 0; i < repeat; i++) 
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->WordRightEnd(); break;
          case MODE_VISUAL: GetSTC()->WordRightEndExtend(); break;
        }
      }
      break;
      
    case 'g': GetSTC()->DocumentStart(); break;
      
    case 'h': 
    case WXK_LEFT:
      VisualExtendLeftLine();
      for (int i = 0; i < repeat; i++) 
      {
        if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) > 0)
        {
          switch (m_Mode)
          {
            case MODE_NORMAL: GetSTC()->CharLeft(); break;
            case MODE_VISUAL: GetSTC()->CharLeftExtend(); break;
          }
        }
      }
      break;
        
    case 'j': 
    case WXK_DOWN:
      VisualExtendRightLine();
      for (int i = 0; i < repeat; i++) 
      {
         if (GetSTC()->GetCurrentLine() < GetSTC()->GetNumberOfLines())
         {
            switch (m_Mode)
            {
              case MODE_NORMAL: GetSTC()->LineDown(); break;
              case MODE_VISUAL: GetSTC()->LineDownExtend(); break;
              case MODE_VISUAL_LINE: 
                GetSTC()->LineDownExtend(); 
                GetSTC()->LineEndExtend(); 
                break;
            }
         }
      }
      break;
        
    case 'k': 
    case WXK_UP:
      VisualExtendLeftLine();
      for (int i = 0; i < repeat; i++) 
      {
        if (GetSTC()->GetCurrentLine() > 0)
        {
          switch (m_Mode)
          {
            case MODE_NORMAL: GetSTC()->LineUp(); break;
            case MODE_VISUAL: GetSTC()->LineUpExtend(); break;
            case MODE_VISUAL_LINE: 
              GetSTC()->LineUpExtend(); 
              GetSTC()->HomeExtend(); 
              break;
          }
        }
      }
      break;
        
    case 'l': 
    case ' ': 
    case WXK_RIGHT:
      VisualExtendRightLine();
      for (int i = 0; i < repeat; i++) 
      {
        if (GetSTC()->GetCurrentPos() < 
            GetSTC()->GetLineEndPosition(GetSTC()->GetCurrentLine()))
        {
          switch (m_Mode)
          {
            case MODE_NORMAL: GetSTC()->CharRight(); break;
            case MODE_VISUAL: GetSTC()->CharRightExtend(); break;
            case MODE_VISUAL_LINE: break;
          }
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
      
    case 'w': 
      VisualExtendRightLine();
      for (int i = 0; i < repeat; i++) 
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->WordRight(); break;
          case MODE_VISUAL: GetSTC()->WordRightExtend(); break;
        }
      }
      break;
      
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

    case '+': 
    case WXK_RETURN:
      VisualExtendLeftLine();
      for (int i = 0; i < repeat; i++) 
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->LineDown(); break;
          case MODE_VISUAL: 
          case MODE_VISUAL_LINE: 
            GetSTC()->LineDownExtend(); 
          break;
        }
        
        for (int j = 1; j < GetSTC()->WrapCount(GetSTC()->GetCurrentLine()); j++)
        {
          switch (m_Mode)
          {
            case MODE_NORMAL: GetSTC()->LineDown(); break;
            case MODE_VISUAL: 
            case MODE_VISUAL_LINE: 
              GetSTC()->LineDownExtend(); break;
          }
        }
      }
      
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) != 
          GetSTC()->GetLineIndentation(GetSTC()->GetCurrentLine()))
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->VCHome(); break;
          case MODE_VISUAL: GetSTC()->VCHomeExtend(); break;
          case MODE_VISUAL_LINE: GetSTC()->HomeExtend(); break;
        }
      }
      break;
        
    case '-': 
      VisualExtendLeftLine();
      for (int i = 0; i < repeat; i++) 
      {
        if (GetSTC()->GetCurrentLine() > 0)
        {
          switch (m_Mode)
          {
            case MODE_NORMAL: GetSTC()->LineUp(); break;
            case MODE_VISUAL: GetSTC()->LineUpExtend(); break;
            case MODE_VISUAL_LINE: 
              GetSTC()->LineUpExtend(); 
              GetSTC()->HomeExtend(); 
              break;
          }
        }
      }
      
      if (GetSTC()->GetColumn(GetSTC()->GetCurrentPos()) != 
          GetSTC()->GetLineIndentation(GetSTC()->GetCurrentLine()))
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->VCHome(); break;
          case MODE_VISUAL: GetSTC()->VCHomeExtend(); break;
          case MODE_VISUAL_LINE: GetSTC()->HomeExtend(); break;
        }
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
      
    
    case '$': 
      switch (m_Mode)
      {
        case MODE_NORMAL: GetSTC()->LineEnd(); break;
        case MODE_VISUAL: GetSTC()->LineEndExtend(); break;
      }
      break;
    
    case '{': 
      VisualExtendLeftLine();
      for (int i = 0; i < repeat; i++)
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->ParaUp(); break;
          case MODE_VISUAL: 
          case MODE_VISUAL_LINE: 
            GetSTC()->ParaUpExtend(); 
            break;
        }
      }
      break;
      
    case '}': 
      VisualExtendRightLine();
      for (int i = 0; i < repeat; i++) 
      {
        switch (m_Mode)
        {
          case MODE_NORMAL: GetSTC()->ParaDown(); break;
          case MODE_VISUAL: 
          case MODE_VISUAL_LINE: 
            GetSTC()->ParaDownExtend(); 
            break;
        }
      }
      break;

    case WXK_CONTROL_B:
    case WXK_PAGEUP:
      for (int i = 0; i < repeat; i++) GetSTC()->PageUp(); 
      break;
    case WXK_CONTROL_E: 
      for (int i = 0; i < repeat; i++) ChangeNumber(true); 
      break;
    case WXK_CONTROL_F:
    case WXK_PAGEDOWN:
      for (int i = 0; i < repeat; i++) GetSTC()->PageDown(); 
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
      
    default:
      return false;
  }
  
  return true;
}

void wxExVi::CommandReg(const wxString& reg)
{
  if (!reg.empty())
  {
    switch ((int)reg[0])
    {
      // calc register
      case '=': GetFrame()->GetExCommand(this, reg); break;
      // clipboard register
      case '\"': Put(true); break;
      // filename register
      case '%':
        if (m_Mode == MODE_INSERT)
        {
          AddText(GetSTC()->GetFileName().GetFullName());
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
            wxLogStatus("?" + reg);
          }
        }
        else
        {
          wxLogStatus("?" + reg);
        }
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

bool wxExVi::InsertMode(const wxString& command)
{
  if (command.empty())
  {
    return false;
  }

  if (command.Contains(wxUniChar(WXK_CONTROL_R) + wxString("=")))
  {
    if (
      command.StartsWith(wxUniChar(WXK_CONTROL_R) + wxString("=")))
    {
      CommandCalc(command);
      return true;
    }
    else
    {
      InsertMode(command.BeforeFirst(wxUniChar(WXK_CONTROL_R)));
      CommandCalc(command.AfterFirst(wxUniChar(WXK_CONTROL_R)));
      return true;
    }
  }
  else if (RegAfter(wxUniChar(WXK_CONTROL_R), command.Mid(0, 2)))
  {
    CommandReg(command.Mid(1, 1));
    InsertMode(command.Mid(2));
    return true;
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
      
    case WXK_CONTROL_R:
      m_InsertText += command;
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
            if (!GetSTC()->GetOvertype())
            {
              for (int i = 1; i <= m_InsertRepeatCount; i++)
              {
                GetSTC()->AddText(rest);
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
          const wxString lc(GetLastCommand() + m_InsertText);
          
          SetLastCommand(lc + wxUniChar(WXK_ESCAPE));
            
          // Record it (if recording is on).
          GetMacros().Record(lc);
          GetMacros().Record(wxUniChar(WXK_ESCAPE));
        }
        
        m_Mode = MODE_NORMAL;
        GetSTC()->SetOvertype(false);
        
        if (!GetSTC()->GetSelectedText().empty())
        {
          GetSTC()->SelectNone();
        }
      break;

    case WXK_RETURN:
        m_InsertText += GetSTC()->GetEOL();
        GetSTC()->NewLine();
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
        m_InsertText += command;
        CommandReg(command);
        return false;
      }
      else
      {
        if (!GetSTC()->GetOvertype())
        {
          GetSTC()->AddText(command);
        }
        
        if (!m_Dot)
        {
          m_InsertText += command;
        }
      }
  }
  
  return true;
}

void wxExVi::MacroRecord(const wxString& text)
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
    const bool result = InsertMode(event.GetUnicodeKey());
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
    event.GetKeyCode() == WXK_TAB ||
    event.GetKeyCode() == WXK_LEFT ||
    event.GetKeyCode() == WXK_DOWN ||
    event.GetKeyCode() == WXK_UP ||
    event.GetKeyCode() == WXK_RIGHT ||
    event.GetKeyCode() == WXK_PAGEUP ||
    event.GetKeyCode() == WXK_PAGEDOWN)
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

void wxExVi::VisualExtendLeftLine()
{
  if (m_Mode == MODE_VISUAL_LINE && GetSTC()->GetSelectedText().empty())
  {
    GetSTC()->LineEnd();
    GetSTC()->HomeExtend();
  }
}

void wxExVi::VisualExtendRightLine()
{
  if (m_Mode == MODE_VISUAL_LINE && GetSTC()->GetSelectedText().empty())
  {
    GetSTC()->Home();
    GetSTC()->LineEndExtend();
  }
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
