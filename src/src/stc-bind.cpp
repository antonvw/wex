////////////////////////////////////////////////////////////////////////////////
// Name:      stc-bind.cpp
// Purpose:   Implementation of class wex::stc method bind_all
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/msgdlg.h>
#include <wx/numdlg.h>
#include <wex/stc.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/frd.h>
#include <wex/item-vector.h>
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

const auto idEdgeClear = wxWindow::NewControlId(); 
const auto idEdgeSet = wxWindow::NewControlId();
const auto idEolDos = wxWindow::NewControlId(3);
const auto idEolUnix = idEolDos + 1;
const auto idEolMac = idEolDos + 2;
const auto idfold_all = wxWindow::NewControlId();
const auto idHex = wxWindow::NewControlId();
const auto idHexDecCalltip = wxWindow::NewControlId();
const auto idLowercase = wxWindow::NewControlId();
const auto idMarginTextHide = wxWindow::NewControlId();
const auto idMarginTextAuthor = wxWindow::NewControlId();
const auto idMarginTextDate = wxWindow::NewControlId();
const auto idMarginTextId = wxWindow::NewControlId();
const auto idMarkerNext = wxWindow::NewControlId(2);
const auto idMarkerPrevious = idMarkerNext + 1;
const auto idOpenLink = wxWindow::NewControlId();
const auto idopen_mime = wxWindow::NewControlId();
const auto idOpenWWW = wxWindow::NewControlId();
const auto idShowProperties = wxWindow::NewControlId();
const auto idToggleFold = wxWindow::NewControlId();
const auto idUnfold_all = wxWindow::NewControlId();
const auto idUppercase = wxWindow::NewControlId();
const auto idZoomIn = wxWindow::NewControlId();
const auto idZoomOut = wxWindow::NewControlId();

void wex::stc::bind_all()
{
  const int accels = 30; // guess max number of entries
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
  entries[i++].Set(wxACCEL_NORMAL, WXK_F9, idfold_all);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F10, idUnfold_all);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F11, idUppercase);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F12, idLowercase);
  entries[i++].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[i++].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[i++].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);
  
  if (m_data.menu().test(stc_data::MENU_DEBUG))
  {
    int j = ID_EDIT_DEBUG_FIRST;

    for (const auto& e : m_frame->get_debug()->debug_entry().get_commands())
    {
      if (!e.control().empty())
      {
        entries[i++].Set(wxACCEL_CTRL, e.control().at(0), j);

        if (i >= accels)
        {
          log("stc-bind") << "too many control accelerators";
          break;
        }
      }
      j++;
    }
  }

  wxAcceleratorTable accel(i, entries);
  SetAcceleratorTable(accel);

  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (!m_vi.is_active())
    {
      if (isalnum(event.GetUnicodeKey()))
      {
        m_adding_chars = true;
      }
    }
    else if (m_vi.mode().insert())
    {
      if (isalnum(event.GetUnicodeKey()))
      {
        m_adding_chars = true;
      }

      m_auto_complete.apply(event.GetUnicodeKey());
    }
    else
    {
      m_adding_chars = false;
    }

    if (m_vi.on_char(event))
    {
      if (
        GetReadOnly() && 
        isalnum(event.GetUnicodeKey()))
      {
        log::status(_("Document is readonly"));
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
      m_lexer.scintilla_lexer() == "hypertext")
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
           (m_lexer.language() == "xml" || m_lexer.is_keyword(match)) &&
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
               log::status("Autocomplete failed");
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
    check_brace();
    m_fold_level = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;});
      
  Bind(wxEVT_LEFT_DCLICK, [=](wxMouseEvent& event) {
    m_margin_text_click = -1;

    if (GetCurLine().Contains("href")) 
    {
      if (link_open(link_t().set(LINK_OPEN_MIME)))
      {
        return;
      }
    }

    if (link_open(link_t().set(LINK_OPEN)))
    {
      return;
    }

    event.Skip();
    });
  
  Bind(wxEVT_LEFT_UP, [=](wxMouseEvent& event) {
    properties_message();
    event.Skip();
    check_brace();
    m_adding_chars = false;
    m_fold_level = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;
      
    if (
      !m_skip &&
      m_frame->get_debug() != nullptr &&
      m_frame->get_debug()->is_active() &&
      matches_one_of(
        get_filename().extension(), 
        m_frame->get_debug()->debug_entry().extensions()))
    {
      const auto word = (!GetSelectedText().empty() ? 
        GetSelectedText().ToStdString() : get_word_at_pos(GetCurrentPos()));

      if (!word.empty() && isalnum(word[0]))
      {
        m_frame->get_debug()->print(word);
      }
    }
    m_skip = false;});
  
  if (m_data.menu().any())
  {
    Bind(wxEVT_RIGHT_UP, [=](wxMouseEvent& event) {
      try
      {
        menu::menu_t style(menu::menu_t().set(menu::IS_POPUP));

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
    m_frame->set_find_focus(this);
    event.Skip();});

  Bind(wxEVT_STC_AUTOCOMP_COMPLETED, [=](wxStyledTextEvent& event) {
    m_auto_complete.activate(event.GetText().ToStdString());});

  Bind(wxEVT_STC_CHARADDED, [=](wxStyledTextEvent& event) {
    event.Skip();
    auto_indentation(event.GetKey());});
    
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

  Bind(wxEVT_STC_DWELLEND, [=](wxStyledTextEvent& event) {
    if (CallTipActive())
    {
      CallTipCancel();
    }});

  // if we support automatic fold, this can be removed,
  // not yet possible for wx3.0. And add wxSTC_AUTOMATICFOLD_CLICK
  // to config_dialog, and SetAutomaticFold.
  Bind(wxEVT_STC_MARGINCLICK, [=](wxStyledTextEvent& event) {
    m_skip = false;
    if (const auto line = LineFromPosition(event.GetPosition());
      event.GetMargin() == m_margin_folding_number)
    {
      if (const auto level = GetFoldLevel(line); (level & wxSTC_FOLDLEVELHEADERFLAG) > 0)
      {
        ToggleFold(line);
      }
      m_margin_text_click = -1;
    }
    else if (event.GetMargin() == m_margin_text_number)
    {
      m_margin_text_click = line;
        
      if (config("blame.id").get(false))
      {
        wex::vcs vcs {{get_filename()}};

        if (std::string margin(MarginGetText(line));
          !margin.empty() && vcs.entry().log(get_filename(), get_word(margin)))
        {
          AnnotationSetText(line, vcs.entry().get_stdout());
        }
        else if (!vcs.entry().get_stderr().empty())
        {
          log("margin") << vcs.entry().get_stderr();
        }
      }
    }
    else if (event.GetMargin() == m_margin_divider_number)
    {
      if (
        m_frame->get_debug() != nullptr &&
        m_frame->get_debug()->is_active())
      {
        m_frame->get_debug()->toggle_breakpoint(line, this);
        m_skip = true;
      }
      else
      {
        event.Skip();
      }
    }
    else
    {
      event.Skip();
    }});

  Bind(wxEVT_STC_MARGIN_RIGHT_CLICK, [=](wxStyledTextEvent& event) {
    if (event.GetMargin() == m_margin_text_number)
    {
      auto* menu = new wex::menu({{idMarginTextHide, "&Hide"}, {}});
      auto* author = menu->AppendCheckItem(idMarginTextAuthor, "&Show Author");
      auto* date = menu->AppendCheckItem(idMarginTextDate, "&Show Date");
      auto* id = menu->AppendCheckItem(idMarginTextId, "&Show Id");

      if (config("blame.author").get(true))
        author->Check();
      if (config("blame.date").get(true))
        date->Check();
      if (config("blame.id").get(true))
        id->Check();

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
        _("Input") + " 1 - " + std::to_string(GetLineCount()) + ":",
        wxEmptyString,
        _("Enter Line Number"),
        m_data.control().line(), // initial value
        1,
        GetLineCount(),
        this)) > 0)
      {
        m_data.control().line(val);
        stc_data(control_data().line(val), this).inject();
      }
    }
    return true;}, wxID_JUMP_TO);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {get_find_string(); event.Skip();}, 
    wxID_FIND);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {get_find_string(); event.Skip();}, 
    wxID_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (SelectionIsRectangle())
    {
      sort_selection(
        this, 
        event.GetId() == wxID_SORT_ASCENDING ? 
          string_sort_t(): string_sort_t().set(STRING_SORT_DESCENDING));
    }
    else if (const auto pos(wxGetNumberFromUser(_("Input") + ":",
      wxEmptyString,
      _("Enter Sort Position"),
      GetCurrentPos() + 1 - PositionFromLine(GetCurrentLine()),
      1,
      GetLineEndPosition(GetCurrentLine()),
      this)); pos > 0)
    {
      sort_selection(
        this, 
        event.GetId() == wxID_SORT_ASCENDING ? 
          string_sort_t(): string_sort_t().set(STRING_SORT_DESCENDING),
        pos - 1);
    }}, wxID_SORT_ASCENDING, wxID_SORT_DESCENDING);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_frame->get_debug()->execute(event.GetId() - ID_EDIT_DEBUG_FIRST, this);}, 
    ID_EDIT_DEBUG_FIRST, ID_EDIT_DEBUG_LAST);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) 
  {
    switch (event.GetInt())
    {
      case stc_file::FILE_LOAD:
        if (get_lexer().scintilla_lexer().empty() && 
          GetLength() < config("max-lines-lexer").get(10000000))
        {
          get_lexer().set(get_filename().lexer());
          config_get();
        }
        
        guess_type_and_modeline();
        log::status(_("Opened")) << get_filename();
        log::verbose("opened", 1) << get_filename();
        fold();
        [[fallthrough]];

      case stc_file::FILE_LOAD_SYNC:
        EmptyUndoBuffer();
        use_modification_markers(true);

        if (!m_data.inject())
        {
          properties_message();
        }
        break;

      case stc_file::FILE_SAVE_AS:
        get_lexer().set(get_filename().lexer());
        SetName(get_filename().string());
        [[fallthrough]];

      case stc_file::FILE_SAVE:
        SetReadOnly(get_filename().is_readonly());
        marker_delete_all_change();
        log::status(_("Saved")) << get_filename();
        log::verbose("saved", 1) << get_filename();
        break;
    }

    if (get_filename().lexer().language() == "xml")
    {
      if (const pugi::xml_parse_result result = 
        pugi::xml_document().load_file(get_filename().string().c_str());
        !result)
      {
        xml_error(get_filename(), &result, this);
      }
    }
  }, ID_EDIT_FILE_ACTION);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    browser_search(GetSelectedText().ToStdString());}, idOpenWWW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    link_open(link_t().set(LINK_OPEN));}, idOpenLink);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const std::string propnames(PropertyNames());
    const lexer_props l;

    std::string properties = (!propnames.empty() ? 
      l.make_section("Current properties"): std::string());
    
    // Add current (global and lexer) properties.  
    for (const auto& it : lexers::get()->properties())
    {
      properties += l.make_key(it.name(), GetProperty(it.name()));
    }

    for (const auto& it : m_lexer.properties())
    {
      properties += l.make_key(it.name(), GetProperty(it.name()));
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

    if (m_entry_dialog == nullptr)
    {
      m_entry_dialog = new stc_entry_dialog(
        properties, 
        std::string(), 
        window_data().
          size({300, 450}).
          button(wxOK).
          title(_("Properties")));
      m_entry_dialog->get_stc()->get_lexer().set(l);
    }
    else
    {
      m_entry_dialog->get_stc()->set_text(properties);
    }
    m_entry_dialog->Show();}, idShowProperties);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSelectedText().length() > 2) return;
    const wxString& caption = _("Enter Control Character");
    if (is_hexmode()) return m_hexmode.control_char_dialog(caption);
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
        ReplaceSelection(std::to_string((char)new_value));
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
    AnnotationSetText(GetCurrentLine(), event.GetString());}, 
    ID_EDIT_DEBUG_VARIABLE);
  
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
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {fold_all();}, idfold_all);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    for (int i = 0; i < GetLineCount(); i++) EnsureVisible(i);}, idUnfold_all);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_data.flags(stc_data::window_t().set(stc_data::WIN_HEX), control_data::XOR).inject();}, idHex);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    config("blame.author").toggle(true);},
    idMarginTextAuthor);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    config("blame.date").toggle(true);},
    idMarginTextDate);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    config("blame.id").toggle(true);},
    idMarginTextId);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(++m_zoom);}, idZoomIn);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(--m_zoom);}, idZoomOut);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frd->set_search_down(true);
    find_next();}, 
    ID_EDIT_FIND_NEXT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    frd->set_search_down(false);
    find_next();}, 
    ID_EDIT_FIND_PREVIOUS);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    link_open(link_t().set(LINK_OPEN_MIME));}, idopen_mime);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const auto level = GetFoldLevel(GetCurrentLine());
    const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetCurrentLine(): GetFoldParent(GetCurrentLine());
    ToggleFold(line_to_fold);}, idToggleFold);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    vcs_execute(m_frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, 
      std::vector< path >{get_filename().data()});},
      ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetReadOnly())
    {
      log::status(_("Document is readonly"));
    }
    else
    {
      if (is_hexmode())
      {
        log::status(_("Not allowed in hex mode"));
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
    reset_margins(margin_t().set(stc::MARGIN_TEXT));
    m_margin_text_click = -1;
    const item_vector& iv(m_config_items);
    SetWrapMode(iv.find<long>(_("stc.Wrap line")));}, 
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
      log::status(_("No markers present"));
    }}, idMarkerNext, idMarkerPrevious);
}

void wex::stc::build_popup_menu(menu& menu)
{
  const auto sel(GetSelectedText().ToStdString());

  if (GetCurrentLine() == 0 && !lexers::get()->get_lexers().empty())
  {
    menu.append({{idShowProperties, _("Properties")}});
  }
    
  if (m_data.menu().test(stc_data::MENU_OPEN_LINK))
  {
    if (sel.empty() && link_open(link_t().set(
      LINK_OPEN_MIME).set(LINK_CHECK)))
    {
      menu.append({{}, {idopen_mime, _("&Preview")}});
    }
    else if (std::string filename; link_open(link_t().set(
      LINK_OPEN).set(LINK_CHECK), &filename))
    {
      menu.append({{}, {idOpenLink, _("Open") + " " + filename}});
    }
  }

  if (GetEdgeMode() == wxSTC_EDGE_MULTILINE)
  {
    menu.append({
      {}, 
      {idEdgeSet, _("Edge Column")}, 
      {idEdgeClear, _("Edge Column Reset")}});
  }

  if (m_data.menu().test(stc_data::MENU_OPEN_WWW) && !sel.empty())
  {
    menu.append({{}, {idOpenWWW, _("&Search")}});
  }
  
  if (m_data.menu().test(stc_data::MENU_DEBUG) &&
    matches_one_of(
      get_filename().extension(), 
      m_frame->get_debug()->debug_entry().extensions()))
  {
    m_frame->get_debug()->add_menu(&menu, true);
  }
  
  if (m_data.menu().test(stc_data::MENU_VCS) &&
      get_filename().file_exists() &&
      vcs::dir_exists(get_filename()))
  {
    menu.append({{}, {get_filename()}});
  }

  if (!m_vi.is_active() && GetTextLength() > 0)
  {
    menu.append({{}, {wxID_FIND}});

    if (!GetReadOnly())
    {
      menu.append({{wxID_REPLACE}});
    }
  }

  menu.append({{}, {menu_item::EDIT}});

  if (!GetReadOnly())
  {
    if (!sel.empty())
    {
      auto* menuSelection = new wex::menu(menu.style(), {
        {idUppercase, _("&Uppercase\tF11")},
        {idLowercase, _("&Lowercase\tF12")}});

      if (get_number_of_lines(sel) > 1)
      {
        menuSelection->append({{}, {
          new wex::menu(menu.style(), 
          {{wxID_SORT_ASCENDING}, {wxID_SORT_DESCENDING}}),
          _("&Sort")}});
      }

      menu.append({{}, {menuSelection, _("&Selection")}});
    }
  }

  if (!GetReadOnly() && (CanUndo() || CanRedo()))
  {
    menu.append({{}});
    if (CanUndo()) menu.append({{wxID_UNDO}});
    if (CanRedo()) menu.append({{wxID_REDO}});
  }

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
     sel.empty() && 
     GetProperty("fold") == "1" &&
     m_lexer.is_ok() &&
    !m_lexer.scintilla_lexer().empty())
  {
    menu.append({{},
      {idToggleFold, _("&Toggle Fold\tCtrl+T")},
      {idfold_all, _("&Fold All Lines\tF9")},
      {idUnfold_all, _("&Unfold All Lines\tF10")}});
  }
}

void wex::stc::check_brace()
{
  if (!config(_("stc.Show match")).get(1))
  {
    return;
  }

  if (is_hexmode())
  {
    m_hexmode.highlight_other();
  }
  else if (!check_brace(GetCurrentPos()))
  {
    check_brace(GetCurrentPos() - 1);
  }
}

bool wex::stc::check_brace(int pos)
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
  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  auto* menu = new wex::menu({
    {idEolDos, "&DOS", menu_item::CHECK},
    {idEolMac, "&MAC", menu_item::CHECK},
    {idEolUnix, "&UNIX", menu_item::CHECK},
    {}});

  auto* hex = menu->AppendCheckItem(idHex, "&HEX");
  
  menu->FindItemByPosition(GetEOLMode())->Check();
  
  if (is_hexmode())
  {
    hex->Check();
  }

  PopupMenu(menu);
  
  delete menu;
}
