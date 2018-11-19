////////////////////////////////////////////////////////////////////////////////
// Name:      stc-bind.cpp
// Purpose:   Implementation of class wex::stc method BindAll
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wex/stc.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/frd.h>
#include <wex/lexer-props.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include <wex/path.h>
#include <wex/stcdlg.h>
#include <wex/tokenizer.h>
#include <wex/util.h>
#include <wex/vcs.h>

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
const int idopen_mime = wxWindow::NewControlId();
const int idOpenWWW = wxWindow::NewControlId();
const int idShowProperties = wxWindow::NewControlId();
const int idToggleFold = wxWindow::NewControlId();
const int idUnfoldAll = wxWindow::NewControlId();
const int idUppercase = wxWindow::NewControlId();
const int idZoomIn = wxWindow::NewControlId();
const int idZoomOut = wxWindow::NewControlId();

void wex::stc::BindAll()
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
    if (!m_vi.is_active())
    {
      if (isalnum(event.GetUnicodeKey()))
      {
        m_AddingChars = true;
      }
    }
    else if (m_vi.mode().insert())
    {
      if (isalnum(event.GetUnicodeKey()))
      {
        m_AddingChars = true;
      }

      m_auto_complete.apply(event.GetUnicodeKey());
    }
    else
    {
      m_AddingChars = false;
    }

    if (m_vi.on_char(event))
    {
      if (
        GetReadOnly() && 
        isalnum(event.GetUnicodeKey()))
      {
        log_status(_("Document is readonly"));
        return;
      }
      if (is_hexmode())
      {
        if (GetOvertype())
        {
          if (m_hexmode.replace(event.GetUnicodeKey()))
          {
            CharRight();
          }
        }
        return;
      }
      if (!m_vi.is_active())
      {
        m_auto_complete.apply(event.GetUnicodeKey());
      }
      event.Skip();
    }
    if (
      event.GetUnicodeKey() == '>' && 
      m_Lexer.scintilla_lexer() == "hypertext")
     {
       if (const auto match_pos = FindText(
             GetCurrentPos() - 1,
             PositionFromLine(GetCurrentLine()),
             "<");
           match_pos != wxSTC_INVALID_POSITION && GetCharAt(match_pos + 1) != '!')
       {
         if (const auto match(get_word_at_pos(match_pos + 1));
            match.find("/") != 0 &&
            GetCharAt(GetCurrentPos() - 2) != '/' &&
           (m_Lexer.language() == "xml" || m_Lexer.is_keyword(match)) &&
           !SelectionIsRectangle())
         {
           if (const std::string add("</" + match + ">"); m_vi.is_active())
           {
             if (const int esc = 27;
               !m_vi.command(add) ||
               !m_vi.command(std::string(1, esc)) ||
               !m_vi.command("%") ||
               !m_vi.command("i"))
             {
               log_status("Autocomplete failed");
             }
           }
           else
           {
             InsertText(GetCurrentPos(), add);
           }
         }
       }
     }});
	  
  auto* frd = find_replace_data::get();
  
  Bind(wxEVT_FIND, [=](wxFindDialogEvent& event) {
    find_next(false);});
    
  Bind(wxEVT_FIND_NEXT, [=](wxFindDialogEvent& event) {
    find_next(false);});

  Bind(wxEVT_FIND_REPLACE, [=](wxFindDialogEvent& event) {
    replace_next(false);});
    
  Bind(wxEVT_FIND_REPLACE_ALL, [=](wxFindDialogEvent& event) {
    replace_all(
      frd->get_find_string(), 
      frd->get_replace_string());});
    
  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    if (is_hexmode())
    {  
      if (
        event.GetKeyCode() == WXK_LEFT ||
        event.GetKeyCode() == WXK_RIGHT)
      {
        m_hexmode.set_pos(event);
      }
    }
    if (event.GetKeyCode() == WXK_BACK)
    {
      m_auto_complete.apply(event.GetKeyCode());
    }
    if (m_vi.on_key_down(event))
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
    
    if (std::string filename; link_open(LINK_OPEN | LINK_CHECK, &filename)) 
    {
      if (!link_open(LINK_OPEN)) 
        event.Skip();
    }
    else if (m_Lexer.scintilla_lexer() != "hypertext" ||
      GetCurLine().Contains("href")) 
    {
      if (!link_open(LINK_OPEN_MIME)) 
        event.Skip();
    }
    else event.Skip();});
  
  Bind(wxEVT_LEFT_UP, [=](wxMouseEvent& event) {
    properties_message();
    event.Skip();
    CheckBrace();
    m_AddingChars = false;
    m_FoldLevel = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;});
  
  if (m_Data.menu().any())
  {
    Bind(wxEVT_RIGHT_UP, [=](wxMouseEvent& event) {
      try
      {
        menu::menu_t style = 0; // otherwise CAN_PASTE already on

        if ( GetReadOnly() || is_hexmode()) style.set(menu::IS_READ_ONLY);
        if (!GetSelectedText().empty())  style.set(menu::IS_SELECTED);
        if ( GetTextLength() == 0)       style.set(menu::IS_EMPTY);
        if ( CanPaste())                 style.set(menu::CAN_PASTE);
          
        menu menu(style);
        build_popup_menu(menu);
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
        log(e) << "menu";
      }});
  }
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    m_Frame->set_find_focus(this);
    event.Skip();});

  Bind(wxEVT_STC_AUTOCOMP_COMPLETED, [=](wxStyledTextEvent& event) {
    m_auto_complete.activate(event.GetText().ToStdString());});

  Bind(wxEVT_STC_CHARADDED, [=](wxStyledTextEvent& event) {
    event.Skip();
    auto_indentation(event.GetKey());});
    
#if wxUSE_DRAG_AND_DROP
  Bind(wxEVT_STC_DO_DROP, [=](wxStyledTextEvent& event) {
    if (is_hexmode() || GetReadOnly())
    {
      event.SetDragResult(wxDragNone);
    }
    event.Skip();});

  Bind(wxEVT_STC_START_DRAG, [=](wxStyledTextEvent& event) {
    if (is_hexmode() || GetReadOnly())
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
  // to config_dialog, and SetAutomaticFold.
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

  Bind(wxEVT_STC_MARGIN_RIGHT_CLICK, [=](wxStyledTextEvent& event) {
    if (event.GetMargin() == m_MarginTextNumber)
    {
      auto* menu = new wxMenu();
      menu->Append(idMarginTextHide, "&Hide");
      PopupMenu(menu);
      delete menu;
    }});

  Bind(wxEVT_STC_UPDATEUI, [=](wxStyledTextEvent& event) {
    event.Skip();
    frame::update_statusbar(this, "PaneInfo");});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Copy();}, wxID_COPY);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Cut();}, wxID_CUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Paste();}, wxID_PASTE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Undo();}, wxID_UNDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Redo();}, wxID_REDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SelectAll();}, wxID_SELECTALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {if (!GetReadOnly() && !is_hexmode()) Clear();}, 
    wxID_DELETE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (is_hexmode())
    {
      m_hexmode.goto_dialog();
    }
    else
    {
      if (static long val; (val = wxGetNumberFromUser(
        _("Input") + wxString::Format(" 1 - %d:", GetLineCount()),
        wxEmptyString,
        _("Enter Line Number"),
        m_Data.control().line(), // initial value
        1,
        GetLineCount(),
        this)) > 0)
      {
        m_Data.control().line(val);
        stc_data(control_data().line(val), this).inject();
      }
    }
    return true;}, wxID_JUMP_TO);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {get_find_string(); event.Skip();}, 
    wxID_FIND);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {get_find_string(); event.Skip();}, 
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
      sort_selection(
        this, 
        string_sort_t().set(
          event.GetId() == wxID_SORT_ASCENDING ? 
            0: STRING_SORT_DESCENDING), 
        pos - 1);
    }}, wxID_SORT_ASCENDING, wxID_SORT_DESCENDING);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Frame->get_debug()->execute(event.GetId() - ID_EDIT_DEBUG_FIRST, this);}, 
    ID_EDIT_DEBUG_FIRST, ID_EDIT_DEBUG_LAST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    browser_search(GetSelectedText().ToStdString());}, idOpenWWW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    link_open(LINK_OPEN);}, idOpenLink);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const std::string propnames(PropertyNames());
    const lexer_props l;

    std::string properties = (!propnames.empty() ? 
      l.make_section("Current properties"): std::string());
    
    // Add current (global and lexer) properties.  
    for (const auto& it : lexers::get()->properties())
    {
      properties += l.make_key(it.GetName(), GetProperty(it.GetName()));
    }

    for (const auto& it : m_Lexer.properties())
    {
      properties += l.make_key(it.GetName(), GetProperty(it.GetName()));
    }

    // Add available properties.
    if (!propnames.empty())
    {
      properties += "\n" + l.make_section("Available properties");

      for (tokenizer tkz(propnames, "\n"); tkz.has_more_tokens(); )
      {
        const auto prop(tkz.get_next_token());
        properties += l.make_key(prop, GetProperty(prop), DescribeProperty(prop));
      }
    }

    if (m_EntryDialog == nullptr)
    {
      m_EntryDialog = new stc_entry_dialog(
        properties, 
        std::string(), 
        window_data().
          size({300, 450}).
          button(wxOK).
          title(_("Properties").ToStdString()));
      m_EntryDialog->stc()->get_lexer().set(l);
    }
    else
    {
      m_EntryDialog->stc()->set_text(properties);
    }
    m_EntryDialog->Show();}, idShowProperties);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSelectedText().length() > 2) return;
    const wxString& caption = _("Enter Control Character");
    if (is_hexmode()) return m_hexmode.control_char_dialog(caption.ToStdString());
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

      if (m_vi.is_active())
      {
        m_vi.command(std::string(buffer, 2));
      }
      else
      {
        AddTextRaw(buffer, 1);
      }
      
      process_char(new_value);
    }
    
    value = new_value;
    }, ID_EDIT_CONTROL_CHAR);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (CallTipActive()) CallTipCancel();
    
    const auto pos = GetCurrentPos();

    if (is_hexmode())
    {
      CallTipShow(pos, m_hexmode.get_info());
      return;
    }

    const auto word = (!GetSelectedText().empty() ? 
      GetSelectedText().ToStdString() : get_word_at_pos(pos));

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
      clipboard_add(stream.str());
    }}, idHexDecCalltip);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {MultiEdgeClearAll();}, idEdgeClear);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    MultiEdgeAddLine(GetColumn(GetCurrentPos()), GetEdgeColour());}, idEdgeSet);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LowerCase();}, idLowercase);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {UpperCase();}, idUppercase);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {FoldAll();}, idFoldAll);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    for (int i = 0; i < GetLineCount(); i++) EnsureVisible(i);}, idUnfoldAll);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Data.flags(stc_data::window_t().set(stc_data::WIN_HEX), control_data::XOR).inject();}, idHex);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(++m_Zoom);}, idZoomIn);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(--m_Zoom);}, idZoomOut);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frd->set_search_down(true);
    find_next();}, 
    ID_EDIT_FIND_NEXT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frd->set_search_down(false);
    find_next();}, 
    ID_EDIT_FIND_PREVIOUS);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {link_open(LINK_OPEN_MIME);}, idopen_mime);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const auto level = GetFoldLevel(GetCurrentLine());
    const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetCurrentLine(): GetFoldParent(GetCurrentLine());
    ToggleFold(line_to_fold);}, idToggleFold);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    vcs_execute(m_Frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, 
      std::vector< path >{get_filename().data()});},
      ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetReadOnly())
    {
      log_status(_("Document is readonly"));
    }
    else
    {
      if (is_hexmode())
      {
        log_status(_("Not allowed in hex mode"));
        return;
      }
      else
      {
        int eol_mode = wxSTC_EOL_LF; // default IDEOL_UNIX
        if (event.GetId() == idEolDos) eol_mode = wxSTC_EOL_CRLF;
        else if (event.GetId() == idEolMac) eol_mode = wxSTC_EOL_CR;
    
        ConvertEOLs(eol_mode);
        SetEOLMode(eol_mode);
        frame::update_statusbar(this, "PaneFileType");
      }
    }}, idEolDos, idEolMac);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    reset_margins(margin_t().set(stc::MARGIN_TEXT));}, 
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
      log_status(_("No markers present"));
    }}, idMarkerNext, idMarkerPrevious);
}

void wex::stc::build_popup_menu(menu& menu)
{
  const auto sel(GetSelectedText().ToStdString());

  if (GetCurrentLine() == 0 && !lexers::get()->get_lexers().empty())
  {
    menu.append(idShowProperties, _("Properties"));
  }
    
  if (m_Data.menu().test(stc_data::MENU_OPEN_LINK))
  {
    if (sel.empty() && link_open(link_t().set(
      LINK_OPEN_MIME).set(LINK_CHECK)))
    {
      menu.append_separator();
      menu.append(idopen_mime, _("&Preview"));
    }
    else if (std::string filename; link_open(link_t().set(
      LINK_OPEN).set(LINK_CHECK), &filename))
    {
      menu.append_separator();
      menu.append(idOpenLink, _("Open") + " " + filename);
    }
  }

  if (GetEdgeMode() == wxSTC_EDGE_MULTILINE)
  {
    menu.append_separator();
    menu.append(idEdgeSet, _("Edge Column"));
    menu.append(idEdgeClear, _("Edge Column Reset"));
  }

  if (m_Data.menu().test(stc_data::MENU_OPEN_WWW) && !sel.empty())
  {
    menu.append_separator();
    menu.append(idOpenWWW, _("&Search"));
  }
  
  if (m_Data.menu().test(stc_data::MENU_DEBUG))
  {
    m_Frame->get_debug()->add_menu(&menu, true);
  }
  
  if (m_Data.menu().test(stc_data::MENU_VCS) &&
      get_filename().file_exists() && sel.empty() &&
      vcs::dir_exists(get_filename()))
  {
    menu.append_separator();
    menu.append_vcs(get_filename());
  }

  if (!m_vi.is_active() && GetTextLength() > 0)
  {
    menu.append_separator();
    menu.append(wxID_FIND);

    if (!GetReadOnly())
    {
      menu.append(wxID_REPLACE);
    }
  }

  menu.append_separator();
  menu.append_edit();

  if (!GetReadOnly())
  {
    if (!sel.empty())
    {
      auto* menuSelection = new wex::menu(menu.style());
      menuSelection->append(idUppercase, _("&Uppercase\tF11"));
      menuSelection->append(idLowercase, _("&Lowercase\tF12"));

      if (get_number_of_lines(sel) > 1)
      {
        auto* menuSort = new wex::menu(menu.style());
        menuSort->append(wxID_SORT_ASCENDING);
        menuSort->append(wxID_SORT_DESCENDING);
        menuSelection->append_separator();
        menuSelection->append_submenu(menuSort, _("&Sort"));
      }

      menu.append_separator();
      menu.append_submenu(menuSelection, _("&Selection"));
    }
  }

  if (!GetReadOnly() && (CanUndo() || CanRedo()))
  {
    menu.append_separator();
    if (CanUndo()) menu.append(wxID_UNDO);
    if (CanRedo()) menu.append(wxID_REDO);
  }

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
     sel.empty() && 
     GetProperty("fold") == "1" &&
     m_Lexer.is_ok() &&
    !m_Lexer.scintilla_lexer().empty())
  {
    menu.append_separator();
    menu.append(idToggleFold, _("&Toggle Fold\tCtrl+T"));
    menu.append(idFoldAll, _("&Fold All Lines\tF9"));
    menu.append(idUnfoldAll, _("&Unfold All Lines\tF10"));
  }
}

void wex::stc::CheckBrace()
{
  if (!config(_("Show match")).get(1))
  {
    return;
  }

  if (is_hexmode())
  {
    m_hexmode.highlight_other();
  }
  else if (!CheckBrace(GetCurrentPos()))
  {
    CheckBrace(GetCurrentPos() - 1);
  }
}

bool wex::stc::CheckBrace(int pos)
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

void wex::stc::filetype_menu()
{
  auto* menu = new wex::menu();

  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  menu->AppendRadioItem(idEolDos, "&DOS");
  menu->AppendRadioItem(idEolMac, "&MAC");
  menu->AppendRadioItem(idEolUnix, "&UNIX");
  menu->append_separator();
  wxMenuItem* hex = menu->AppendCheckItem(idHex, "&HEX");
  
  menu->FindItemByPosition(GetEOLMode())->Check();
  
  if (is_hexmode())
  {
    hex->Check();
  }

  PopupMenu(menu);
  
  delete menu;
}
