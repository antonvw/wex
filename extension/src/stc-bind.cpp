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
#include <wx/extension/log.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include <wx/extension/path.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

const int idEdgeClear = wxWindow::NewControlId(); 
const int idEdgeSet = wxWindow::NewControlId();
const int idEolDos = wxWindow::NewControlId(3);
const int idEolUnix = idEolDos + 1;
const int idEolMac = idEolDos + 2;
const int idFoldAll = wxWindow::NewControlId();
const int idHex = wxWindow::NewControlId();
const int idHexDecCalltip = wxWindow::NewControlId();
const int idLowercase = wxWindow::NewControlId();
const int idMarginTextHide = wxWindow::NewControlId();
const int idMarkerNext = wxWindow::NewControlId(2);
const int idMarkerPrevious = idMarkerNext + 1;
const int idOpenLink = wxWindow::NewControlId();
const int idOpenMIME = wxWindow::NewControlId();
const int idOpenWWW = wxWindow::NewControlId();
const int idShowProperties = wxWindow::NewControlId();
const int idToggleFold = wxWindow::NewControlId();
const int idUnfoldAll = wxWindow::NewControlId();
const int idUppercase = wxWindow::NewControlId();
const int idZoomIn = wxWindow::NewControlId();
const int idZoomOut = wxWindow::NewControlId();

void wxExSTC::BindAll()
{
  const int accels = 20; // take max number of entries
  wxAcceleratorEntry entries[accels];

  int i = 0;

  entries[i++].Set(wxACCEL_CTRL, (int)'Z', wxID_UNDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'Y', wxID_REDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'D', idHexDecCalltip);
  entries[i++].Set(wxACCEL_CTRL, (int)'K', ID_EDIT_CONTROL_CHAR);
  entries[i++].Set(wxACCEL_CTRL, '=', idZoomIn);
  entries[i++].Set(wxACCEL_CTRL, '-', idZoomOut);
  entries[i++].Set(wxACCEL_CTRL, '9', idMarkerNext);
  entries[i++].Set(wxACCEL_CTRL, '0', idMarkerPrevious);
  entries[i++].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F7, wxID_SORT_ASCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F8, wxID_SORT_DESCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F9, idFoldAll);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F10, idUnfoldAll);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F11, idUppercase);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F12, idLowercase);
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
       if (const auto match_pos = FindText(
             GetCurrentPos() - 1,
             PositionFromLine(GetCurrentLine()),
             "<");
           match_pos != wxSTC_INVALID_POSITION && GetCharAt(match_pos + 1) != '!')
       {
         if (const auto match(GetWordAtPos(match_pos + 1));
            match.find("/") != 0 &&
            GetCharAt(GetCurrentPos() - 2) != '/' &&
           (m_Lexer.GetLanguage() == "xml" || m_Lexer.IsKeyword(match)) &&
           !SelectionIsRectangle())
         {
           if (const std::string add("</" + match + ">"); m_vi.GetIsActive())
           {
             if (const int esc = 27;
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
	  
  auto* frd = wxExFindReplaceData::Get();
  
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
    
    if (std::string filename; LinkOpen(LINK_OPEN | LINK_CHECK, &filename)) 
    {
      if (!LinkOpen(LINK_OPEN)) 
        event.Skip();
    }
    else if (m_Lexer.GetScintillaLexer() != "hypertext" ||
      GetCurLine().Contains("href")) 
    {
      if (!LinkOpen(LINK_OPEN_MIME)) 
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
          if (wxMenuItem* item = menu.FindItemByPosition(menu.GetMenuItemCount() - 1);
            item->IsSeparator())
          {
            menu.Delete(item->GetId());
          }
          PopupMenu(&menu);
        }
      }
      catch (std::exception& e)
      {
        wxExLog(e) << "menu";
      }});
  }
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    m_Frame->SetFindFocus(this);
    event.Skip();});

  Bind(wxEVT_STC_AUTOCOMP_COMPLETED, [=](wxStyledTextEvent& event) {
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
    if (const auto line = LineFromPosition(event.GetPosition());
      event.GetMargin() == m_MarginFoldingNumber)
    {
      if (const auto level = GetFoldLevel(line); (level & wxSTC_FOLDLEVELHEADERFLAG) > 0)
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
      auto* menu = new wxMenu();
      menu->Append(idMarginTextHide, "&Hide");
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
      if (long val; (val = wxGetNumberFromUser(
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
    if (long pos; (pos = wxGetNumberFromUser(_("Input") + ":",
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
    wxExBrowserSearch(GetSelectedText().ToStdString());}, idOpenWWW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    LinkOpen(LINK_OPEN);}, idOpenLink);

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
    
      for (wxExTokenizer tkz(propnames, "\n"); tkz.HasMoreTokens(); )
      {
        const auto prop(tkz.GetNextToken());
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
    m_EntryDialog->Show();}, idShowProperties);

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

    const auto word = (!GetSelectedText().empty() ? 
      GetSelectedText().ToStdString() : GetWordAtPos(pos));

    if (word.empty()) 
    {
      return;
    }

    std::stringstream stream;

    if (const int c = word[0]; c < 32 || c > 125)
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
      catch (std::exception&)
      {
        base10_ok = false;
      }

      try
      {
        base16_val = std::stol(word, nullptr, 16);
      }
      catch (std::exception&)
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
    }}, idHexDecCalltip);
  
#if wxCHECK_VERSION(3,1,0)
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {MultiEdgeClearAll();}, idEdgeClear);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    MultiEdgeAddLine(GetColumn(GetCurrentPos()), GetEdgeColour());}, idEdgeSet);
#endif

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LowerCase();}, idLowercase);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {UpperCase();}, idUppercase);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {FoldAll();}, idFoldAll);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {for (int i = 0; i < GetLineCount(); i++) EnsureVisible(i);}, idUnfoldAll);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {m_Data.Flags(STC_WIN_HEX, DATA_XOR).Inject();}, idHex);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(++m_Zoom);}, idZoomIn);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(--m_Zoom);}, idZoomOut);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); FindNext(true);}, ID_EDIT_FIND_NEXT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); FindNext(false);}, ID_EDIT_FIND_PREVIOUS);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LinkOpen(LINK_OPEN_MIME);}, idOpenMIME);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const auto level = GetFoldLevel(GetCurrentLine());
    const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetCurrentLine(): GetFoldParent(GetCurrentLine());
    ToggleFold(line_to_fold);}, idToggleFold);
    
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
        int eol_mode = wxSTC_EOL_LF; // default IDEOL_UNIX
        if (event.GetId() == idEolDos) eol_mode = wxSTC_EOL_CRLF;
        else if (event.GetId() == idEolMac) eol_mode = wxSTC_EOL_CR;
    
        ConvertEOLs(eol_mode);
        SetEOLMode(eol_mode);
        wxExFrame::UpdateStatusBar(this, "PaneFileType");
      }
    }}, idEolDos, idEolMac);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {ResetMargins(STC_MARGIN_TEXT);}, 
    idMarginTextHide);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    auto line = (event.GetId() == idMarkerNext ? 
      wxStyledTextCtrl::MarkerNext(GetCurrentLine() + 1, 0xFFFF):
      wxStyledTextCtrl::MarkerPrevious(GetCurrentLine() - 1, 0xFFFF));
    if (line == -1)
    {
      line = (event.GetId() == idMarkerNext ?
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
    }}, idMarkerNext, idMarkerPrevious);
}

void wxExSTC::BuildPopupMenu(wxExMenu& menu)
{
  const auto sel(GetSelectedText().ToStdString());

  if (GetCurrentLine() == 0 && !wxExLexers::Get()->GetLexers().empty())
  {
    menu.Append(idShowProperties, _("Properties"));
  }
    
  if (m_Data.Menu() & STC_MENU_OPEN_LINK)
  {
    if (sel.empty() && LinkOpen(LINK_OPEN_MIME | LINK_CHECK))
    {
      menu.AppendSeparator();
      menu.Append(idOpenMIME, _("&Preview"));
    }
    else if (std::string filename; LinkOpen(LINK_OPEN | LINK_CHECK, &filename))
    {
      menu.AppendSeparator();
      menu.Append(idOpenLink, _("Open") + " " + filename);
    }
  }

#if wxCHECK_VERSION(3,1,0)
  if (GetEdgeMode() == wxSTC_EDGE_MULTILINE)
  {
    menu.AppendSeparator();
    menu.Append(idEdgeSet, _("Edge Column"));
    menu.Append(idEdgeClear, _("Edge Column Reset"));
  }
#endif

  if (m_Data.Menu() & STC_MENU_OPEN_WWW && !sel.empty())
  {
    menu.AppendSeparator();
    menu.Append(idOpenWWW, _("&Search"));
  }
  
  if (m_Data.Menu() & STC_MENU_DEBUG)
  {
    m_Frame->GetDebug()->AddMenu(&menu, true);
  }
  
  if ((m_Data.Menu() & STC_MENU_VCS) &&
       GetFileName().FileExists() && sel.empty() &&
       wxExVCS::DirExists(GetFileName()))
  {
    menu.AppendSeparator();
    menu.AppendVCS(GetFileName());
  }

  if (!m_vi.GetIsActive() && GetTextLength() > 0)
  {
    menu.AppendSeparator();
    menu.Append(wxID_FIND);

    if (!GetReadOnly())
    {
      menu.Append(wxID_REPLACE);
    }
  }

  menu.AppendSeparator();
  menu.AppendEdit();

  if (!GetReadOnly())
  {
    if (!sel.empty())
    {
      auto* menuSelection = new wxExMenu(menu.GetStyle());
      menuSelection->Append(idUppercase, _("&Uppercase\tF11"));
      menuSelection->Append(idLowercase, _("&Lowercase\tF12"));

      if (wxExGetNumberOfLines(sel) > 1)
      {
        auto* menuSort = new wxExMenu(menu.GetStyle());
        menuSort->Append(wxID_SORT_ASCENDING);
        menuSort->Append(wxID_SORT_DESCENDING);
        menuSelection->AppendSeparator();
        menuSelection->AppendSubMenu(menuSort, _("&Sort"));
      }

      menu.AppendSeparator();
      menu.AppendSubMenu(menuSelection, _("&Selection"));
    }
  }

  if (!GetReadOnly() && (CanUndo() || CanRedo()))
  {
    menu.AppendSeparator();
    if (CanUndo()) menu.Append(wxID_UNDO);
    if (CanRedo()) menu.Append(wxID_REDO);
  }

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
     sel.empty() && 
     GetProperty("fold") == "1" &&
     m_Lexer.IsOk() &&
    !m_Lexer.GetScintillaLexer().empty())
  {
    menu.AppendSeparator();
    menu.Append(idToggleFold, _("&Toggle Fold\tCtrl+T"));
    menu.Append(idFoldAll, _("&Fold All Lines\tF9"));
    menu.Append(idUnfoldAll, _("&Unfold All Lines\tF10"));
  }
}

void wxExSTC::CheckBrace()
{
  if (HexMode())
  {
    m_HexMode.HighlightOther();
  }
  else if (!CheckBrace(GetCurrentPos()))
  {
    CheckBrace(GetCurrentPos() - 1);
  }
}

bool wxExSTC::CheckBrace(int pos)
{
  if (const auto brace_match = BraceMatch(pos); brace_match != wxSTC_INVALID_POSITION)
  {
    BraceHighlight(pos, brace_match);
    return true;
  }
  else
  {
    BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
    return false;
  }
}

void wxExSTC::FileTypeMenu()
{
  auto* menu = new wxMenu();

  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  menu->AppendRadioItem(idEolDos, "&DOS");
  menu->AppendRadioItem(idEolMac, "&MAC");
  menu->AppendRadioItem(idEolUnix, "&UNIX");
  menu->AppendSeparator();
  wxMenuItem* hex = menu->AppendCheckItem(idHex, "&HEX");
  
  menu->FindItemByPosition(GetEOLMode())->Check();
  
  if (HexMode())
  {
    hex->Check();
  }

  PopupMenu(menu);
  
  delete menu;
}
