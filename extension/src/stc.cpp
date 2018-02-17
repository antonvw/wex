////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <easylogging++.h>
#include <wx/app.h>
#include <wx/config.h>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wx/settings.h>
#include <wx/extension/stc.h>
#include <wx/extension/ctags.h>
#include <wx/extension/debug.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/frd.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include <wx/extension/path.h>
#include <wx/extension/printing.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

#if wxUSE_GUI

enum
{
  LINK_CHECK         = 0x0001,
  LINK_OPEN          = 0x0002,
  LINK_OPEN_BROWSER  = 0x0004,
};

int wxExSTC::m_Zoom = -1;

wxExSTC::wxExSTC(const std::string& text, const wxExSTCData& data)
  : wxExSTC(wxExPath(), data)
{
  if (!text.empty())
  {
    HexMode() ? m_HexMode.AppendText(text): SetText(text);
    GuessType();
  }
  
  m_Data.Inject();
}

wxExSTC::wxExSTC(const wxExPath& filename, const wxExSTCData& data)
  : wxStyledTextCtrl(
      data.Window().Parent(),
      data.Window().Id(), 
      data.Window().Pos(), 
      data.Window().Size(), 
      data.Window().Style(), 
      data.Window().Name())
  , m_Data(this, data)
  , m_vi(this)
  , m_File(this, data.Window().Name())
  , m_Link(this)
  , m_HexMode(wxExHexMode(this))
  , m_Frame(dynamic_cast<wxExManagedFrame*>(wxTheApp->GetTopWindow()))
  , m_Lexer(this)
{
  if (wxConfig::Get()->ReadBool("AllowSync", true)) Sync();
  
  if (!wxExLexers::Get()->GetLexers().empty())
  {
    m_DefaultFont = wxConfigBase::Get()->ReadObject(
      _("Default font"), wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT));
  }
  
#ifdef __WXMSW__
  SetEOLMode(wxSTC_EOL_CRLF);
#elif __WXGTK__
  SetEOLMode(wxSTC_EOL_LF);
#else
  SetEOLMode(wxSTC_EOL_CR);
#endif

  SetAdditionalCaretsBlink(true);
  SetAdditionalCaretsVisible(true);
  SetAdditionalSelectionTyping(true);  
  SetBackSpaceUnIndents(true);
  SetMouseDwellTime(1000);
  SetMarginType(m_MarginLineNumber, wxSTC_MARGIN_NUMBER);
  SetMarginType(m_MarginDividerNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_MarginFoldingNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginMask(m_MarginFoldingNumber, wxSTC_MASK_FOLDERS);
  SetMarginSensitive(m_MarginFoldingNumber, true);
  SetMultiPaste(wxSTC_MULTIPASTE_EACH);
  SetMultipleSelection(true);
  
  m_vi.GetCTags()->AutoCompletePrepare(this);

  if (m_Zoom == -1)
  {
    m_Zoom = GetZoom();
  }
  else
  {
    SetZoom(m_Zoom);
  }

  // we have our own popup
#if wxCHECK_VERSION(3,1,1)
  UsePopUp(wxSTC_POPUP_NEVER);
#else
  UsePopUp(false);
#endif

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

  ConfigGet();

  Fold();
  
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
    m_AutoComplete.clear();

    if (m_vi.GetCTags()->Filter(
      event.GetText().ToStdString(), 
      m_AutoCompleteFilter))
    {
      VLOG(9) << "filter: " << m_AutoCompleteFilter.Get();
    }

    if (m_vi.GetIsActive())
    {
      const std::string command(event.GetText().substr(
        m_AutoComplete.size()));

      if (!command.empty() && !m_vi.Command(command))
      {
        wxLogStatus("Autocomplete failed");
      }}});
    
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
    if (event.GetMargin() == m_MarginFoldingNumber)
    {
      const int line = LineFromPosition(event.GetPosition());
      const int level = GetFoldLevel(line);
      if ((level & wxSTC_FOLDLEVELHEADERFLAG) > 0)
      {
        ToggleFold(line);
      }
    }});

  Bind(wxEVT_STC_UPDATEUI, [=](wxStyledTextEvent& event) {
    event.Skip();
    wxExFrame::UpdateStatusBar(this, "PaneInfo");});
    
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

      CheckAutoComp(event.GetUnicodeKey());
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
        CheckAutoComp(event.GetUnicodeKey());
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
	  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Copy();}, wxID_COPY);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Cut();}, wxID_CUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Paste();}, wxID_PASTE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Undo();}, wxID_UNDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Redo();}, wxID_REDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SelectAll();}, wxID_SELECTALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {if (!GetReadOnly() && !HexMode()) Clear();}, wxID_DELETE);
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
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); event.Skip();}, wxID_FIND);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); event.Skip();}, wxID_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {m_Frame->GetDebug()->Execute(event.GetId() - ID_EDIT_DEBUG_FIRST, this);}, ID_EDIT_DEBUG_FIRST, ID_EDIT_DEBUG_LAST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {wxExBrowserSearch(GetSelectedText().ToStdString());}, ID_EDIT_OPEN_WWW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LinkOpen(LINK_OPEN);}, ID_EDIT_OPEN_LINK);
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

  if (filename.GetStat().IsOk())
  {
    Open(filename, data);
  }
  else
  {
    m_File.FileLoad(filename);
    m_Lexer.Set(filename.GetLexer(), true); // allow fold
  }
}

void wxExSTC::BuildPopupMenu(wxExMenu& menu)
{
  const std::string sel = GetSelectedText().ToStdString();

  if (GetCurrentLine() == 0 && !wxExLexers::Get()->GetLexers().empty())
  {
    menu.Append(ID_EDIT_SHOW_PROPERTIES, _("Properties"));
  }
    
  if (m_Data.Menu() & STC_MENU_OPEN_LINK)
  {
    std::string filename;

    if (LinkOpen(LINK_OPEN_BROWSER | LINK_CHECK))
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_OPEN_BROWSER, _("&Open In Browser"));
    }
    else if (LinkOpen(LINK_OPEN | LINK_CHECK, &filename))
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_OPEN_LINK, _("Open") + " " + filename);
    }
  }

#if wxCHECK_VERSION(3,1,0)
  if (GetEdgeMode() == wxSTC_EDGE_MULTILINE)
  {
    menu.AppendSeparator();
    menu.Append(ID_EDIT_EDGE_SET, _("Edge Column"));
    menu.Append(ID_EDIT_EDGE_CLEAR, _("Edge Column Reset"));
  }
#endif

  if (m_Data.Menu() & STC_MENU_OPEN_WWW && !sel.empty())
  {
    menu.AppendSeparator();
    menu.Append(ID_EDIT_OPEN_WWW, _("&Search"));
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
      wxExMenu* menuSelection = new wxExMenu(menu.GetStyle());
      menuSelection->Append(ID_EDIT_UPPERCASE, _("&Uppercase\tF11"));
      menuSelection->Append(ID_EDIT_LOWERCASE, _("&Lowercase\tF12"));

      if (wxExGetNumberOfLines(sel) > 1)
      {
        wxExMenu* menuSort = new wxExMenu(menu.GetStyle());
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
    menu.Append(ID_EDIT_TOGGLE_FOLD, _("&Toggle Fold\tCtrl+T"));
    menu.Append(ID_EDIT_FOLD_ALL, _("&Fold All Lines\tF9"));
    menu.Append(ID_EDIT_UNFOLD_ALL, _("&Unfold All Lines\tF10"));
  }
}

bool wxExSTC::CanCut() const
{
  return wxStyledTextCtrl::CanCut() && !GetReadOnly() && !HexMode();
}

bool wxExSTC::CanPaste() const
{
  return wxStyledTextCtrl::CanPaste() && !GetReadOnly() && !HexMode();
}

void wxExSTC::CheckAutoComp(const wxUniChar& c)
{
  if (!m_UseAutoComplete || 
      !wxConfigBase::Get()->ReadBool(_("Autocomplete"), true) ||
      SelectionIsRectangle())
  {
    return;
  }
  
  if (wxExIsCodewordSeparator(GetCharAt(GetCurrentPos() - 1)))
  {
    m_AutoComplete = c;
  }
  else
  {
    if (c == '.')
    {
      m_AutoComplete.clear();
    }
    else if (isspace(c))
    {
      if (m_AutoComplete.size() > 2)
      {
        m_AutoCompleteInserts.emplace(m_AutoComplete);
      }

      m_AutoComplete.clear();
      return;
    }
    else
    {
      m_AutoComplete += c;
    }

    if (m_AutoCompleteFilter.Active())
    {
      const std::string comp(m_vi.GetCTags()->AutoComplete(
        m_AutoComplete, m_AutoCompleteFilter));

      if (!comp.empty())
      {
        AutoCompSetSeparator(m_vi.GetCTags()->Separator());
        AutoCompShow(m_AutoComplete.length() - 1, comp);
        AutoCompSetSeparator(' ');
      }
      else
      {
        AutoCompCancel();
      }
    }
    else if (m_AutoComplete.length() > 2) // Only autocompletion for large words
    {
      const std::string comp(m_vi.GetCTags()->AutoComplete(m_AutoComplete));
      bool show = false;

      if (!comp.empty())
      {
        AutoCompSetSeparator(m_vi.GetCTags()->Separator());
        AutoCompShow(m_AutoComplete.length() - 1, comp);
        AutoCompSetSeparator(' ');
        show = true;
      }

      const int min_size = 3;

      if (m_Lexer.KeywordStartsWith(m_AutoComplete))
      {
        const std::string comp(
          m_Lexer.GetKeywordsString(-1, min_size, m_AutoComplete));
          
        if (!comp.empty())
        {
          AutoCompShow(m_AutoComplete.length() - 1, comp);
          show = true;
        }
      }

      if (!m_AutoCompleteInserts.empty())
      {
        const std::string comp(wxExGetStringSet(
          m_AutoCompleteInserts, min_size, m_AutoComplete));
          
        if (!comp.empty())
        {
          AutoCompShow(m_AutoComplete.length() - 1, comp);
          show = true;
        }
      }

      if (!show)
      {
        AutoCompCancel();
      }
    }
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
  const int brace_match = BraceMatch(pos);

  if (brace_match != wxSTC_INVALID_POSITION)
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

void wxExSTC::Clear()
{
  if (m_vi.GetIsActive() && GetSelectedText().empty())
  {
    (void)m_vi.Command(std::string(1, WXK_DELETE));
  }
  else
  {
    wxStyledTextCtrl::Clear();
  }
}

void wxExSTC::ClearDocument(bool set_savepoint)
{
  SetReadOnly(false);
  
  ClearAll();
  
  if (set_savepoint)
  {
    EmptyUndoBuffer();
    SetSavePoint();
  }
}

void wxExSTC::Copy()
{
  if (CanCopy()) 
  {
    wxStyledTextCtrl::Copy();
  }
}

void wxExSTC::Cut()
{
  if (CanCut()) 
  {
    if (m_vi.GetIsActive())
    {
      const wxCharBuffer b(GetSelectedTextRaw());
      m_vi.SetRegistersDelete(std::string(b.data(), b.length() - 1));
      m_vi.SetRegisterYank(std::string(b.data(), b.length() - 1));
    }
  
    wxStyledTextCtrl::Cut();
  }
}
  
bool wxExSTC::FileReadOnlyAttributeChanged()
{
  SetReadOnly(GetFileName().IsReadOnly()); // does not return anything
  wxLogStatus(_("Readonly attribute changed"));

  return true;
}

void wxExSTC::FileTypeMenu()
{
  wxMenu* menu = new wxMenu();

  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  menu->AppendRadioItem(ID_EDIT_EOL_DOS, "&DOS");
  menu->AppendRadioItem(ID_EDIT_EOL_MAC, "&MAC");
  menu->AppendRadioItem(ID_EDIT_EOL_UNIX, "&UNIX");
  menu->AppendSeparator();
  wxMenuItem* hex = menu->AppendCheckItem(ID_EDIT_HEX, "&HEX");
  
  menu->FindItemByPosition(GetEOLMode())->Check();
  
  if (HexMode())
  {
    hex->Check();
  }

  PopupMenu(menu);
  
  delete menu;
}

bool wxExSTC::FindNext(bool find_next)
{
  return FindNext(
    GetFindString(),
    -1,
    find_next);
}

bool wxExSTC::FindNext(
  const std::string& text, 
  int find_flags,
  bool find_next)
{
  if (text.empty())
  {
    return false;
  }

  static bool recursive = false;
  static int start_pos, end_pos;

  if (find_next)
  {
    if (recursive) start_pos = 0;
    else
    {
      start_pos = GetCurrentPos();
      end_pos = GetTextLength();
    }
  }
  else
  {
    if (recursive) start_pos = GetTextLength();
    else
    {
      start_pos = GetCurrentPos();
      if (GetSelectionStart() != -1)
        start_pos = GetSelectionStart();

      end_pos = 0;
    }
  }

  SetTargetStart(start_pos);
  SetTargetEnd(end_pos);
  SetSearchFlags(find_flags);

  if (SearchInTarget(text) == -1)
  {
    wxExFrame::StatusText(
      wxExGetFindResult(text, find_next, recursive), std::string());
    
    bool found = false;
    
    if (!recursive)
    {
      recursive = true;
      found = FindNext(text, find_flags, find_next);
      recursive = false;

      if (!found)
      {
        VLOG(9) << GetFileName().GetFullName() << " text: " << text << " not found";
      }
    }
    
    return found;
  }
  else
  {
    if (!recursive)
    {
      wxLogStatus(wxEmptyString);
    }
    
    recursive = false;

    if (m_vi.Mode().Normal())
    {
        SetSelection(GetTargetStart(), GetTargetEnd());
    }
    else if (m_vi.Mode().Visual())
    {
      if (find_next)
        m_vi.VisualExtend(GetSelectionStart(), GetTargetEnd());
      else
        m_vi.VisualExtend(GetTargetStart(), GetSelectionEnd());
    }
      
    EnsureVisible(LineFromPosition(GetTargetStart()));
    EnsureCaretVisible();

    VLOG(9) << GetFileName().GetFullName() << " found text: " << text;

    return true;
  }
}

void wxExSTC::Fold(bool foldall)
{
  if (
     GetProperty("fold") == "1" &&
     m_Lexer.IsOk() &&
    !m_Lexer.GetScintillaLexer().empty())
  {
    SetMarginWidth(m_MarginFoldingNumber, 
      wxConfigBase::Get()->ReadLong(_("Folding"), 0));
    SetFoldFlags(
      wxConfigBase::Get()->ReadLong(_("Fold flags"),
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | 
        wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
        
    if (
      foldall || 
      GetLineCount() > wxConfigBase::Get()->ReadLong(_("Auto fold"), 0))
    {
      FoldAll();
    }
  }
  else
  {
    SetMarginWidth(m_MarginFoldingNumber, 0);
  }
}
  
void wxExSTC::FoldAll()
{
  if (GetProperty("fold") != "1") return;

  const int current_line = GetCurrentLine();
  const bool xml = (m_Lexer.GetLanguage() == "xml");

  int line = 0;
  while (line < GetLineCount())
  {
    const int level = GetFoldLevel(line);
    const int last_child_line = GetLastChild(line, level);
    
    if (xml && (
        level == wxSTC_FOLDLEVELBASE + wxSTC_FOLDLEVELHEADERFLAG))
    {
      line++;
    }
    else if (last_child_line > line + 1)
    {
      if (GetFoldExpanded(line)) ToggleFold(line);
      line = last_child_line + 1;
    }
    else
    {
      line++;
    }
  }

  GotoLine(current_line);
}

const std::string wxExSTC::GetEOL() const
{
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR: return "\r";
    case wxSTC_EOL_CRLF: return "\r\n";
    case wxSTC_EOL_LF: return "\n";
    default: wxFAIL; break;
  }

  return "\r\n";
}

// Cannot be const because of GetSelectedText (not const in 2.9.4).
const std::string wxExSTC::GetFindString()
{
  const std::string selection = GetSelectedText().ToStdString();

  if (!selection.empty() && wxExGetNumberOfLines(selection) == 1)
  {
    bool alnum = true;
    
    // If regexp is true, then only use selected text if text does not
    // contain special regexp characters.
    if (GetSearchFlags() & wxSTC_FIND_REGEXP)
    {
      for (size_t i = 0; i < selection.size() && alnum; i++)
      {
        if (
          !isalnum(selection[i]) && 
           selection[i] != ' ' && 
           selection[i] != '.' && 
           selection[i] != '-' && 
           selection[i] != '_')
        {
          alnum = false;
        }
      }
    }

    if (alnum)
    {  
      wxExFindReplaceData::Get()->SetFindString(selection);
    }
  }

  return wxExFindReplaceData::Get()->GetFindString();
}


const std::string wxExSTC::GetWordAtPos(int pos) const
{
  const int word_start = 
    const_cast< wxExSTC * >( this )->WordStartPosition(pos, true);
  const int word_end = 
    const_cast< wxExSTC * >( this )->WordEndPosition(pos, true);

  if (word_start == word_end && word_start < GetTextLength())
  {
    const std::string word = 
      const_cast< wxExSTC * >( this )->GetTextRange(word_start, word_start + 1).ToStdString();

    return !isspace(word[0]) ? word: std::string();
  }
  else
  {
    return const_cast< wxExSTC * >( this )->GetTextRange(word_start, word_end).ToStdString();
  }
}

void wxExSTC::GuessType()
{
  // Get a small sample from this document to detect the file mode.
  const auto length = (!HexMode() ? GetTextLength(): m_HexMode.GetBuffer().size());
  const auto sample_size = (length > 255 ? 255: length);
  const std::string text((!HexMode() ? GetTextRange(0, sample_size).ToStdString(): 
    m_HexMode.GetBuffer().substr(0, sample_size)));
  const std::string text2((!HexMode() ? GetTextRange(length - sample_size, length).ToStdString(): 
    m_HexMode.GetBuffer().substr(length - sample_size, sample_size)));

  std::vector<std::string> v;  
  
  // If we have a modeline comment.
  if (
    m_vi.GetIsActive() && 
     (wxExMatch("vi: *(set [a-z0-9:= ]+)", text, v) > 0 ||
      wxExMatch("vi: *(set [a-z0-9:= ]+)", text2, v) > 0))
  {
    if (!m_vi.Command(":" + v[0] + "*")) // add * to indicate modelin
    {
      wxLogStatus("Could not apply vi settings");
    }
  }

  if      (text.find("\r\n") != std::string::npos) SetEOLMode(wxSTC_EOL_CRLF);
  else if (text.find("\n") != std::string::npos)   SetEOLMode(wxSTC_EOL_LF);
  else if (text.find("\r") != std::string::npos)   SetEOLMode(wxSTC_EOL_CR);
  else return; // do nothing

#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
}

bool wxExSTC::LinkOpen()
{
  return LinkOpen(LINK_OPEN | LINK_OPEN_BROWSER);
}

bool wxExSTC::LinkOpen(int mode, std::string* filename)
{
  const std::string sel = GetSelectedText().ToStdString();

  if (sel.size() > 200 || sel.find('\n') != std::string::npos)
  {
    return false;
  }

  const std::string text = (!sel.empty() ? sel: GetCurLine().ToStdString());

  if (mode & LINK_OPEN_BROWSER)
  {
    const wxExPath path(m_Link.GetPath(text, 
      wxExControlData().Line(sel.empty() ? -1 : -2)));
    
    if (!path.Path().string().empty()) 
    {
      if (!(mode & LINK_CHECK)) 
      {
        wxLaunchDefaultBrowser(path.Path().string());
      }

      return true;
    }
  }

  if (mode & LINK_OPEN)
  {
    wxExControlData data;

    const wxExPath path(m_Link.GetPath(text, data));
    
    if (!path.Path().string().empty()) 
    {
      if (filename != nullptr)
      {
        *filename = path.GetFullName();
      }
      else if (m_Frame != nullptr)
      {
        m_Frame->OpenFile(path, data);
      }
      else
      {
        Open(path, data);
      }

      return true;
    }
  }
  
  return false;
}

bool wxExSTC::MarkerDeleteAllChange()
{
  if (!wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    return false;
  }
  
  MarkerDeleteAll(m_MarkerChange.GetNo());
  
  return true;
}
  
void wxExSTC::MarkModified(const wxStyledTextEvent& event)
{
  if (!wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    return;
  }
  
  UseModificationMarkers(false);
  
  const int line = LineFromPosition(event.GetPosition());
  
  if (event.GetModificationType() & wxSTC_PERFORMED_UNDO)
  {
    if (event.GetLinesAdded() == 0)
    {
      MarkerDelete(line, m_MarkerChange.GetNo());
    }
    else
    {
      for (int i = 0; i < abs(event.GetLinesAdded()); i++)
      {
        MarkerDelete(line + 1, m_MarkerChange.GetNo());
      }
    }
      
    if (!IsModified())
    {
      MarkerDeleteAllChange();
    }
  }
  else if (
    (event.GetModificationType() & wxSTC_MOD_INSERTTEXT) ||
    (event.GetModificationType() & wxSTC_MOD_DELETETEXT))
  {
    if (event.GetLinesAdded() <= 0)
    {
      MarkerAdd(line, m_MarkerChange.GetNo());
    }
    else
    {
      for (int i = 0; i < event.GetLinesAdded(); i++)
      {
        MarkerAdd(line + i, m_MarkerChange.GetNo());
      }
    }
  }
    
  UseModificationMarkers(true);
}
  

void wxExSTC::OnIdle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    m_File.CheckSync() &&
    // the readonly flags bit of course can differ from file actual readonly mode,
    // therefore add this check
    !(m_Data.Flags() & STC_WIN_READ_ONLY) &&
      GetFileName().GetStat().IsReadOnly() != GetReadOnly())
  {
    FileReadOnlyAttributeChanged();
  }
}

void wxExSTC::OnStyledText(wxStyledTextEvent& event)
{
  MarkModified(event); 
  event.Skip();
}

bool wxExSTC::Open(const wxExPath& filename, const wxExSTCData& data)
{
  if (GetFileName() != filename && !m_File.FileLoad(filename))
  {
    return false;
  }

  m_Data = wxExSTCData(data).Window(wxExWindowData().Name(filename.Path().string()));
  m_Data.Inject();

  if (m_Frame != nullptr)
  {
    m_Frame->SetRecentFile(filename);
  }

  return true;
}

void wxExSTC::Paste()
{
  if (CanPaste())
  {
    wxStyledTextCtrl::Paste();
  }
}

bool wxExSTC::PositionRestore()
{
  if (m_vi.Mode().Visual())
  {
    SetCurrentPos(m_SavedPos);
  }
  else if (m_SavedSelectionStart != -1 && m_SavedSelectionEnd != -1)
  {
    SetSelection(m_SavedSelectionStart, m_SavedSelectionEnd);
    SetCurrentPos(m_SavedSelectionStart);
  }
  else if (m_SavedPos != -1)
  {
    SetSelection(m_SavedPos, m_SavedPos);
    SetCurrentPos(m_SavedPos);
  }
  else
  {
    return false;
  }
  
  EnsureCaretVisible();
  
  return true;
}
  
void wxExSTC::PositionSave()
{
  m_SavedPos = GetCurrentPos();

  if (!m_vi.Mode().Visual())
  {
    m_SavedSelectionStart = GetSelectionStart();  
    m_SavedSelectionEnd = GetSelectionEnd();
  }
}

#if wxUSE_PRINTING_ARCHITECTURE
void wxExSTC::Print(bool prompt)
{
  wxPrintData* data = wxExPrinting::Get()->GetHtmlPrinter()->GetPrintData();
  wxExPrinting::Get()->GetPrinter()->GetPrintDialogData().SetPrintData(*data);
  wxExPrinting::Get()->GetPrinter()->Print(this, new wxExPrintout(this), prompt);
}
#endif

#if wxUSE_PRINTING_ARCHITECTURE
void wxExSTC::PrintPreview(wxPreviewFrameModalityKind kind)
{
  wxPrintPreview* preview = new wxPrintPreview(
    new wxExPrintout(this), 
    new wxExPrintout(this));

  if (!preview->Ok())
  {
    delete preview;
    wxExSTCEntryDialog("There was a problem previewing.\n"
      "Perhaps your current printer is not set correctly?").ShowModal();
    return;
  }

  wxPreviewFrame* frame = new wxPreviewFrame(
    preview,
    this,
    wxExPrintCaption(GetName().ToStdString()));

  frame->InitializeWithModality(kind);
  frame->Show();
}
#endif

void wxExSTC::PropertiesMessage(long flags)
{
  wxExLogStatus(GetFileName(), flags);
  
#if wxUSE_STATUSBAR
  if (flags != STAT_SYNC)
  {
    wxExFrame::UpdateStatusBar(this, "PaneFileType");
    wxExFrame::UpdateStatusBar(this, "PaneLexer");
    wxExFrame::UpdateStatusBar(this, "PaneMode");
  }
  
  wxExFrame::UpdateStatusBar(this, "PaneInfo");
#endif

  if (!(flags & STAT_SYNC) && m_Frame != nullptr)
  {
    const wxString file = GetName() + 
      (GetReadOnly() ? " [" + _("Readonly") + "]": wxString());
    
    m_Frame->SetTitle(!file.empty() ? file: wxTheApp->GetAppName());
  }
}

int wxExSTC::ReplaceAll(
  const std::string& find_text,
  const std::string& replace_text)
{
  int selection_from_end = 0;

  if (SelectionIsRectangle() || wxExGetNumberOfLines(GetSelectedText().ToStdString()) > 1)
  {
    TargetFromSelection();
    selection_from_end = GetLength() - GetTargetEnd();
  }
  else
  {
    SetTargetStart(0);
    SetTargetEnd(GetLength());
  }

  int nr_replacements = 0;
  SetSearchFlags(-1);
  BeginUndoAction();

  while (SearchInTarget(find_text) != -1)
  {
    bool skip_replace = false;

    // Check that the target is within the rectangular selection.
    // If not just continue without replacing.
    if (SelectionIsRectangle())
    {
      const int line = LineFromPosition(GetTargetStart());
      const int start_pos = GetLineSelStartPosition(line);
      const int end_pos = GetLineSelEndPosition(line);
      const int length = GetTargetEnd() - GetTargetStart();

      if (start_pos == wxSTC_INVALID_POSITION ||
          end_pos == wxSTC_INVALID_POSITION ||
          GetTargetStart() < start_pos ||
          GetTargetStart() + length > end_pos)
      {
        skip_replace = true;
      }
    }

    if (!skip_replace)
    {
      if (HexMode())
      {
        m_HexMode.ReplaceTarget(replace_text);
      }
      else
      {
        wxExFindReplaceData::Get()->UseRegEx() ?
          ReplaceTargetRE(replace_text):
          ReplaceTarget(replace_text);
      }

      nr_replacements++;
    }

    SetTargetStart(GetTargetEnd());
    SetTargetEnd(GetLength() - selection_from_end);
    
    if (GetTargetStart() >= GetTargetEnd())
    {
      break;
    }
  }

  EndUndoAction();

  wxLogStatus(_("Replaced: %d occurrences of: %s"),
    nr_replacements, find_text.c_str());

  return nr_replacements;
}

bool wxExSTC::ReplaceNext(bool find_next)
{
  return ReplaceNext(
    wxExFindReplaceData::Get()->GetFindString(),
    wxExFindReplaceData::Get()->GetReplaceString(),
    -1,
    find_next);
}

bool wxExSTC::ReplaceNext(
  const std::string& find_text, 
  const std::string& replace_text,
  int find_flags,
  bool find_next)
{
  if (!GetSelectedText().empty())
  {
    TargetFromSelection();
  }
  else
  {
    SetTargetStart(GetCurrentPos());
    SetTargetEnd(GetLength());
    SetSearchFlags(find_flags);
    if (SearchInTarget(find_text) == -1) return false;
  }

  if (HexMode())
  {
    m_HexMode.ReplaceTarget(replace_text);
  }
  else
  {
    wxExFindReplaceData::Get()->UseRegEx() ?
      ReplaceTargetRE(replace_text):
      ReplaceTarget(replace_text);
  }

  FindNext(find_text, find_flags, find_next);
  
  return true;
}

 
void wxExSTC::ResetMargins(wxExSTCMarginFlags flags)
{
  if (flags & STC_MARGIN_FOLDING) SetMarginWidth(m_MarginFoldingNumber, 0);
  if (flags & STC_MARGIN_DIVIDER) SetMarginWidth(m_MarginLineNumber, 0);
  if (flags & STC_MARGIN_LINENUMBER) SetMarginWidth(m_MarginDividerNumber, 0);
}

void wxExSTC::SelectNone()
{
  if (SelectionIsRectangle())
  {
    // SetSelection does not work.
    CharRight();
    CharLeft();
  }
  else
  {
    // The base styledtextctrl version uses scintilla, sets caret at 0.
    SetSelection(GetCurrentPos(), GetCurrentPos());
  }
}

bool wxExSTC::SetIndicator(const wxExIndicator& indicator, int start, int end)
{
  if (!wxExLexers::Get()->IndicatorIsLoaded(indicator))
  {
    return false;
  }

  SetIndicatorCurrent(indicator.GetNo());
  IndicatorFillRange(start, end - start);
  
  return true;
}

void wxExSTC::SetSearchFlags(int flags)
{
  if (flags == -1)
  {
    flags = 0;
    
    wxExFindReplaceData* frd = wxExFindReplaceData::Get();
    if (frd->UseRegEx()) 
    {
      flags |= wxSTC_FIND_REGEXP;
#if wxCHECK_VERSION(3,1,1)
      flags |= wxSTC_FIND_CXX11REGEX;
#endif
    }
    if (frd->MatchWord()) flags |= wxSTC_FIND_WHOLEWORD;
    if (frd->MatchCase()) flags |= wxSTC_FIND_MATCHCASE;
  }

  wxStyledTextCtrl::SetSearchFlags(flags);
}

void wxExSTC::SetText(const std::string& value)
{
  ClearDocument();

  AddTextRaw((const char *)value.c_str(), value.length());

  DocumentStart();

  // Do not allow the text specified to be undone.
  EmptyUndoBuffer();
}

void wxExSTC::ShowLineNumbers(bool show)
{
  SetMarginWidth(m_MarginLineNumber, show ? wxConfigBase::Get()->ReadLong(_("Line number"), 0): 0);
}

void wxExSTC::Sync(bool start)
{
  start ?
    Bind(wxEVT_IDLE, &wxExSTC::OnIdle, this):
    (void)Unbind(wxEVT_IDLE, &wxExSTC::OnIdle, this);
}

void wxExSTC::Undo()
{
  wxStyledTextCtrl::Undo();
  m_HexMode.Undo();
}

void wxExSTC::UseAutoComplete(bool use)
{
  m_UseAutoComplete = use;
}

void wxExSTC::UseModificationMarkers(bool use)
{
  if (use)
    Bind(wxEVT_STC_MODIFIED, &wxExSTC::OnStyledText, this);
  else
    Unbind(wxEVT_STC_MODIFIED, &wxExSTC::OnStyledText, this);
}

void wxExSTC::WordLeftRectExtend() 
{
  const int repeat = GetCurrentPos() - WordStartPosition(GetCurrentPos(), false);
  
  for (int i = 0; i < repeat ; i++)
  {
    CharLeftRectExtend();
  }
}

void wxExSTC::WordRightRectExtend() 
{
  const int repeat = WordEndPosition(GetCurrentPos(), false) - GetCurrentPos();
  
  for (int i = 0; i < repeat; i++)
  {
    CharRightRectExtend();
  }
}
#endif // wxUSE_GUI
