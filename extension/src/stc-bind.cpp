////////////////////////////////////////////////////////////////////////////////
// Name:      stc-bind.cpp
// Purpose:   Implementation of class wxExSTC method BindAll
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/extension/stc.h>
#include <wx/extension/debug.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include <wx/extension/path.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <easylogging++.h>

const int ID_EDIT_MARGIN_TEXT_HIDE = wxWindow::NewControlId();

void wxExSTC::BindAll()
{
  const int accels = 20; // take max number of entries
  wxAcceleratorEntry entries[accels];

  int i = 0;

  entries[i++].Set(wxACCEL_CTRL, (int)'Z', wxID_UNDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'Y', wxID_REDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'D', ID_EDIT_HEX_DEC_CALLTIP);
  entries[i++].Set(wxACCEL_CTRL, (int)'K', ID_EDIT_CONTROL_CHAR);
  entries[i++].Set(wxACCEL_CTRL, '=', ID_EDIT_ZOOM_IN);
  entries[i++].Set(wxACCEL_CTRL, '-', ID_EDIT_ZOOM_OUT);
  entries[i++].Set(wxACCEL_CTRL, '9', ID_EDIT_MARKER_NEXT);
  entries[i++].Set(wxACCEL_CTRL, '0', ID_EDIT_MARKER_PREVIOUS);
  entries[i++].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F7, wxID_SORT_ASCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F8, wxID_SORT_DESCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F9, ID_EDIT_FOLD_ALL);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F10, ID_EDIT_UNFOLD_ALL);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F11, ID_EDIT_UPPERCASE);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F12, ID_EDIT_LOWERCASE);
  entries[i++].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[i++].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[i++].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);

  wxAcceleratorTable accel(i, entries);
  SetAcceleratorTable(accel);

  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (!m_vi.GetIsActive())
    {
      if (isalnum(event.GetUnicodeKey()))
      {
        m_AddingChars = true;
      }
    }
    else if (m_vi.Mode().Insert())
    {
      if (isalnum(event.GetUnicodeKey()))
      {
        m_AddingChars = true;
      }

      m_AutoComplete.Apply(event.GetUnicodeKey());
    }
    else
    {
      m_AddingChars = false;
    }

    if (m_vi.OnChar(event))
    {
      if (
        GetReadOnly() && 
        isalnum(event.GetUnicodeKey()))
      {
        wxLogStatus(_("Document is readonly"));
        return;
      }
      if (HexMode())
      {
        if (GetOvertype())
        {
          if (m_HexMode.Replace(event.GetUnicodeKey()))
          {
            CharRight();
          }
        }
        return;
      }
      if (!m_vi.GetIsActive())
      {
        m_AutoComplete.Apply(event.GetUnicodeKey());
      }
      event.Skip();
    }
    if (
      event.GetUnicodeKey() == '>' && 
      m_Lexer.GetScintillaLexer() == "hypertext")
     {
       const int match_pos = FindText(
         GetCurrentPos() - 1,
         PositionFromLine(GetCurrentLine()),
         "<");
       if (match_pos != wxSTC_INVALID_POSITION && GetCharAt(match_pos + 1) != '!')
       {
         const std::string match(GetWordAtPos(match_pos + 1));

         if (
            match.find("/") != 0 &&
            GetCharAt(GetCurrentPos() - 2) != '/' &&
           (m_Lexer.GetLanguage() == "xml" || m_Lexer.IsKeyword(match)) &&
           !SelectionIsRectangle())
         {
           const std::string add("</" + match + ">");
           if (m_vi.GetIsActive())
           {
             const int esc = 27;
             if (
               !m_vi.Command(add) ||
               !m_vi.Command(std::string(1, esc)) ||
               !m_vi.Command("%") ||
               !m_vi.Command("i"))
             {
               wxLogStatus("Autocomplete failed");
             }
           }
           else
           {
             InsertText(GetCurrentPos(), add);
           }
         }
       }
     }});
	  
  wxExFindReplaceData* frd = wxExFindReplaceData::Get();
  
  Bind(wxEVT_FIND, [=](wxFindDialogEvent& event) {
    frd->SetFindString(frd->GetFindString());
    FindNext(frd->SearchDown());});
    
  Bind(wxEVT_FIND_NEXT, [=](wxFindDialogEvent& event) {
    frd->SetFindString(frd->GetFindString());
    FindNext(frd->SearchDown());});

  Bind(wxEVT_FIND_REPLACE, [=](wxFindDialogEvent& event) {
    ReplaceNext(wxExFindReplaceData::Get()->SearchDown());});
    
  Bind(wxEVT_FIND_REPLACE_ALL, [=](wxFindDialogEvent& event) {
    ReplaceAll(
      frd->GetFindString(), 
      frd->GetReplaceString());});
    
  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    if (HexMode())
    {  
      if (
        event.GetKeyCode() == WXK_LEFT ||
        event.GetKeyCode() == WXK_RIGHT)
      {
        m_HexMode.SetPos(event);
      }
    }
    if (event.GetKeyCode() == WXK_BACK)
    {
      m_AutoComplete.Apply(event.GetKeyCode());
    }
    if (m_vi.OnKeyDown(event))
    {
      event.Skip();
    }});

  Bind(wxEVT_KEY_UP, [=](wxKeyEvent& event) {
    event.Skip();
    CheckBrace();
    m_FoldLevel = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;});
      
  Bind(wxEVT_LEFT_DCLICK, [=](wxMouseEvent& event) {
    m_MarginTextClick = -1;
    std::string filename;
    if (LinkOpen(LINK_OPEN | LINK_CHECK, &filename)) 
    {
      if (!LinkOpen(LINK_OPEN)) 
        event.Skip();
    }
    else if (m_Lexer.GetScintillaLexer() != "hypertext" ||
      GetCurLine().Contains("href")) 
    {
      if (!LinkOpen(LINK_OPEN_BROWSER)) 
        event.Skip();
    }
    else event.Skip();});
  
  Bind(wxEVT_LEFT_UP, [=](wxMouseEvent& event) {
    PropertiesMessage();
    event.Skip();
    CheckBrace();
    m_AddingChars = false;
    m_FoldLevel = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;});
  
  if (m_Data.Menu() != STC_MENU_NONE)
  {
    Bind(wxEVT_RIGHT_UP, [=](wxMouseEvent& event) {
      try
      {
        int style = 0; // otherwise CAN_PASTE already on
        if ( GetReadOnly() || HexMode()) style |= wxExMenu::MENU_IS_READ_ONLY;
        if (!GetSelectedText().empty())  style |= wxExMenu::MENU_IS_SELECTED;
        if ( GetTextLength() == 0)       style |= wxExMenu::MENU_IS_EMPTY;
        if ( CanPaste())                 style |= wxExMenu::MENU_CAN_PASTE;
        wxExMenu menu(style);
        BuildPopupMenu(menu);
        if (menu.GetMenuItemCount() > 0)
        {
          // If last item is a separator, delete it.
          wxMenuItem* item = menu.FindItemByPosition(menu.GetMenuItemCount() - 1);
          if (item->IsSeparator())
          {
            menu.Delete(item->GetId());
          }
          PopupMenu(&menu);
        }
      }
      catch (std::exception& e)
      {
        LOG(ERROR) << e.what();
      }});
  }
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    m_Frame->SetFindFocus(this);
    event.Skip();});

  Bind(wxEVT_STC_AUTOCOMP_SELECTION, [=](wxStyledTextEvent& event) {
    m_AutoComplete.Activate(event.GetText().ToStdString());});
    
  Bind(wxEVT_STC_CHARADDED, [=](wxStyledTextEvent& event) {
    event.Skip();
    AutoIndentation(event.GetKey());});
    
#if wxUSE_DRAG_AND_DROP
  Bind(wxEVT_STC_DO_DROP, [=](wxStyledTextEvent& event) {
    if (HexMode() || GetReadOnly())
    {
      event.SetDragResult(wxDragNone);
    }
    event.Skip();});

  Bind(wxEVT_STC_START_DRAG, [=](wxStyledTextEvent& event) {
    if (HexMode() || GetReadOnly())
    {
      event.SetDragAllowMove(false);
    }
    event.Skip();});
#endif    
    
  Bind(wxEVT_STC_DWELLEND, [=](wxStyledTextEvent& event) {
    if (CallTipActive())
    {
      CallTipCancel();
    }});

  // if we support automatic fold, this can be removed,
  // not yet possible for wx3.0. And add wxSTC_AUTOMATICFOLD_CLICK
  // to configdialog, and SetAutomaticFold.
  Bind(wxEVT_STC_MARGINCLICK, [=](wxStyledTextEvent& event) {
    const int line = LineFromPosition(event.GetPosition());
    if (event.GetMargin() == m_MarginFoldingNumber)
    {
      const int level = GetFoldLevel(line);
      if ((level & wxSTC_FOLDLEVELHEADERFLAG) > 0)
      {
        ToggleFold(line);
      }
      m_MarginTextClick = -1;
    }
    else
    {
      m_MarginTextClick = line;
    }});

#if wxCHECK_VERSION(3,1,0)
  Bind(wxEVT_STC_MARGIN_RIGHT_CLICK, [=](wxStyledTextEvent& event) {
#else
  Bind(wxEVT_STC_MARGINCLICK, [=](wxStyledTextEvent& event) {
#endif
    if (event.GetMargin() == m_MarginTextNumber)
    {
      wxMenu* menu = new wxMenu();
      menu->Append(ID_EDIT_MARGIN_TEXT_HIDE, "&Hide");
      PopupMenu(menu);
      delete menu;
    }});

  Bind(wxEVT_STC_UPDATEUI, [=](wxStyledTextEvent& event) {
    event.Skip();
    wxExFrame::UpdateStatusBar(this, "PaneInfo");});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Copy();}, wxID_COPY);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Cut();}, wxID_CUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Paste();}, wxID_PASTE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Undo();}, wxID_UNDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Redo();}, wxID_REDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SelectAll();}, wxID_SELECTALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {if (!GetReadOnly() && !HexMode()) Clear();}, 
    wxID_DELETE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (HexMode())
    {
      m_HexMode.GotoDialog();
    }
    else
    {
      long val;
      if ((val = wxGetNumberFromUser(
        _("Input") + wxString::Format(" 1 - %d:", GetLineCount()),
        wxEmptyString,
        _("Enter Line Number"),
        m_Data.Control().Line(), // initial value
        1,
        GetLineCount(),
        this)) > 0)
      {
        wxExSTCData(wxExControlData().Line(val), this).Inject();
      }
    }
    return true;}, wxID_JUMP_TO);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); event.Skip();}, 
    wxID_FIND);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); event.Skip();}, 
    wxID_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    long pos;
    if ((pos = wxGetNumberFromUser(_("Input") + ":",
      wxEmptyString,
      _("Enter Sort Position"),
      GetCurrentPos() + 1 - PositionFromLine(GetCurrentLine()),
      1,
      GetLineEndPosition(GetCurrentLine()),
      this)) > 0)
    {
      wxExSortSelection(this, event.GetId() == wxID_SORT_ASCENDING ? STRING_SORT_ASCENDING: STRING_SORT_DESCENDING, pos - 1);
    }}, wxID_SORT_ASCENDING, wxID_SORT_DESCENDING);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Frame->GetDebug()->Execute(event.GetId() - ID_EDIT_DEBUG_FIRST, this);}, ID_EDIT_DEBUG_FIRST, ID_EDIT_DEBUG_LAST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExBrowserSearch(GetSelectedText().ToStdString());}, ID_EDIT_OPEN_WWW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    LinkOpen(LINK_OPEN);}, ID_EDIT_OPEN_LINK);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const std::string propnames(PropertyNames());
    std::string properties = (!propnames.empty() ? "[Current properties]\n": std::string());
    
    // Add current (global and lexer) properties.  
    for (const auto& it : wxExLexers::Get()->GetProperties())
    {
      properties += it.GetName() + "=" + GetProperty(it.GetName()) + "\n";
    }
    for (const auto& it : m_Lexer.GetProperties())
    {
      properties += it.GetName() + "=" + GetProperty(it.GetName()) + "\n";
    }
    // Add available properties.
    if (!propnames.empty())
    {
      properties += "\n[Available properties]\n";
      wxExTokenizer tkz(propnames, "\n");
    
      while (tkz.HasMoreTokens())
      {
        const std::string prop(tkz.GetNextToken());
        const std::string description(DescribeProperty(prop));
        properties += prop + 
          (!GetProperty(prop).empty() ? "=" + GetProperty(prop).ToStdString(): std::string()) + 
          (!description.empty() ? ": " + description: std::string()) + "\n";
      }
    }
    if (m_EntryDialog == nullptr)
    {
      m_EntryDialog = new wxExSTCEntryDialog(
        properties, 
        std::string(), 
        wxExWindowData().Button(wxOK).Title(_("Properties").ToStdString()));
      m_EntryDialog->GetSTC()->GetLexer().Set("props");
    }
    else
    {
      m_EntryDialog->GetSTC()->SetText(properties);
    }
    m_EntryDialog->Show();}, ID_EDIT_SHOW_PROPERTIES);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSelectedText().length() > 2) return;
    const wxString& caption = _("Enter Control Character");
    if (HexMode()) return m_HexMode.ControlCharDialog(caption.ToStdString());
    if (GetReadOnly())
    {
      if (GetSelectedText().length() == 1)
      {
        const wxUniChar value = GetSelectedText().GetChar(0);
        wxMessageBox(
          wxString::Format("hex: %x dec: %d", value, value), 
          _("Control Character"));
      }
      return;
    }

    static int value = ' '; // don't use 0 as default as nullptr is not handled
    if (GetSelectedText().length() == 1) value = GetSelectedText().GetChar(0);
    int new_value;
    if ((new_value = (int)wxGetNumberFromUser(_("Input") + " 0 - 255:",
      wxEmptyString, caption, value, 0, 255, this)) < 0) return;

    if (GetSelectedText().length() == 1)
    {
      if (value != new_value)
      {
        ReplaceSelection(wxString::Format("%c", (wxUniChar)new_value));
      }

      SetSelection(GetCurrentPos(), GetCurrentPos() + 1);
    }
    else
    {
      char buffer[2];
      buffer[0] = (char)new_value;
      buffer[1] = 0;

      if (m_vi.GetIsActive())
      {
        m_vi.Command(std::string(buffer, 2));
      }
      else
      {
        AddTextRaw(buffer, 1);
      }
      
      ProcessChar(new_value);
    }
    
    value = new_value;
    }, ID_EDIT_CONTROL_CHAR);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (CallTipActive()) CallTipCancel();
    
    const auto pos = GetCurrentPos();

    if (HexMode())
    {
      CallTipShow(pos, m_HexMode.GetInfo());
      return;
    }

    const std::string word = (!GetSelectedText().empty() ? 
      GetSelectedText().ToStdString() : GetWordAtPos(pos));

    if (word.empty()) 
    {
      return;
    }

    const int c = word[0];
    std::stringstream stream;

    if (c < 32 || c > 125)
    {
      stream << "bin: " << c;
    }
    else
    {
      long base10_val, base16_val;
      bool base10_ok = true;
      bool base16_ok = true;

      try
      {
        base10_val = std::stol(word);
        base10_ok = (base10_val != 0);
      }
      catch (std::exception& e)
      {
        base10_ok = false;
      }

      try
      {
        base16_val = std::stol(word, nullptr, 16);
      }
      catch (std::exception& e)
      {
        base16_ok = false;
      }

      if (base10_ok || base16_ok)
      {
        if      ( base10_ok && !base16_ok) 
          stream << "hex: " << std::hex << base10_val;
        else if (!base10_ok &&  base16_ok) 
          stream << "dec: " << base16_val;
        else if ( base10_ok &&  base16_ok) 
          stream << "dec: " << base16_val << " hex: " << std::hex << base10_val;
      }
    }

    if (!stream.str().empty())
    {
      CallTipShow(pos, stream.str());
      wxExClipboardAdd(stream.str());
    }}, ID_EDIT_HEX_DEC_CALLTIP);
  
#if wxCHECK_VERSION(3,1,0)
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {MultiEdgeClearAll();}, ID_EDIT_EDGE_CLEAR);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    MultiEdgeAddLine(GetColumn(GetCurrentPos()), GetEdgeColour());}, ID_EDIT_EDGE_SET);
#endif

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LowerCase();}, ID_EDIT_LOWERCASE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {UpperCase();}, ID_EDIT_UPPERCASE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {FoldAll();}, ID_EDIT_FOLD_ALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {for (int i = 0; i < GetLineCount(); i++) EnsureVisible(i);}, ID_EDIT_UNFOLD_ALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {m_Data.Flags(STC_WIN_HEX, DATA_XOR).Inject();}, ID_EDIT_HEX);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(++m_Zoom);}, ID_EDIT_ZOOM_IN);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(--m_Zoom);}, ID_EDIT_ZOOM_OUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); FindNext(true);}, ID_EDIT_FIND_NEXT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); FindNext(false);}, ID_EDIT_FIND_PREVIOUS);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LinkOpen(LINK_OPEN_BROWSER);}, ID_EDIT_OPEN_BROWSER);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const int level = GetFoldLevel(GetCurrentLine());
    const int line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetCurrentLine(): GetFoldParent(GetCurrentLine());
    ToggleFold(line_to_fold);}, ID_EDIT_TOGGLE_FOLD);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExVCSExecute(m_Frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, 
      std::vector< wxExPath >{GetFileName().Path()});},
      ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetReadOnly())
    {
      wxLogStatus(_("Document is readonly"));
    }
    else
    {
      if (HexMode())
      {
        wxLogStatus(_("Not allowed in hex mode"));
        return;
      }
      else
      {
        int eol_mode = wxSTC_EOL_LF; // default ID_EDIT_EOL_UNIX
        
        switch (event.GetId())
        {
          case ID_EDIT_EOL_DOS: eol_mode = wxSTC_EOL_CRLF; break;
          case ID_EDIT_EOL_MAC: eol_mode = wxSTC_EOL_CR; break;
        }
    
        ConvertEOLs(eol_mode);
        SetEOLMode(eol_mode);
#if wxUSE_STATUSBAR
        wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
      }
    }}, ID_EDIT_EOL_DOS, ID_EDIT_EOL_MAC);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {ResetMargins(STC_MARGIN_TEXT);}, 
    ID_EDIT_MARGIN_TEXT_HIDE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    int line = (event.GetId() == ID_EDIT_MARKER_NEXT ? 
      wxStyledTextCtrl::MarkerNext(GetCurrentLine() + 1, 0xFFFF):
      wxStyledTextCtrl::MarkerPrevious(GetCurrentLine() - 1, 0xFFFF));
    if (line == -1)
    {
      line = (event.GetId() == ID_EDIT_MARKER_NEXT ?
        wxStyledTextCtrl::MarkerNext(0, 0xFFFF):
        wxStyledTextCtrl::MarkerPrevious(GetLineCount() - 1, 0xFFFF));
    }
    if (line != -1)
    {
      GotoLine(line);
    }
    else
    {
      wxLogStatus(_("No markers present"));
    }}, ID_EDIT_MARKER_NEXT, ID_EDIT_MARKER_PREVIOUS);
}
