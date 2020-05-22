////////////////////////////////////////////////////////////////////////////////
// Name:      stc/bind.cpp
// Purpose:   Implementation of class wex::stc method bind_all
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wex/accelerators.h>
#include <wex/beautify.h>
#include <wex/bind.h>
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
#include <wex/stc.h>
#include <wex/stcdlg.h>
#include <wex/tokenizer.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wx/app.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>

const auto idEdgeClear      = wxWindow::NewControlId();
const auto idEdgeSet        = wxWindow::NewControlId();
const auto idfold_all       = wxWindow::NewControlId();
const auto idBeautify       = wxWindow::NewControlId();
const auto idHex            = wxWindow::NewControlId();
const auto idHexDecCalltip  = wxWindow::NewControlId();
const auto idLowercase      = wxWindow::NewControlId();
const auto idMarkerNext     = wxWindow::NewControlId(2);
const auto idMarkerPrevious = idMarkerNext + 1;
const auto idOpenLink       = wxWindow::NewControlId();
const auto idopen_mime      = wxWindow::NewControlId();
const auto idOpenWWW        = wxWindow::NewControlId();
const auto idShowProperties = wxWindow::NewControlId();
const auto idToggleFold     = wxWindow::NewControlId();
const auto idUnfold_all     = wxWindow::NewControlId();
const auto idUppercase      = wxWindow::NewControlId();
const auto idZoomIn         = wxWindow::NewControlId();
const auto idZoomOut        = wxWindow::NewControlId();

void wex::stc::bind_all()
{
  accelerators(
    {{wxACCEL_CTRL, 'D', idHexDecCalltip},
     {wxACCEL_CTRL, 'K', ID_EDIT_CONTROL_CHAR},
     {wxACCEL_CTRL, 'Y', wxID_REDO},
     {wxACCEL_CTRL, 'Z', wxID_UNDO},
     {wxACCEL_CTRL, '=', idZoomIn},
     {wxACCEL_CTRL, '-', idZoomOut},
     {wxACCEL_CTRL, '0', idMarkerPrevious},
     {wxACCEL_CTRL, '9', idMarkerNext},
     {wxACCEL_CTRL, WXK_INSERT, wxID_COPY},
     {wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT},
     {wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS},
     {wxACCEL_NORMAL, WXK_F7, wxID_SORT_ASCENDING},
     {wxACCEL_NORMAL, WXK_F8, wxID_SORT_DESCENDING},
     {wxACCEL_NORMAL, WXK_F9, idfold_all},
     {wxACCEL_NORMAL, WXK_F10, idUnfold_all},
     {wxACCEL_NORMAL, WXK_F11, idUppercase},
     {wxACCEL_NORMAL, WXK_F12, idLowercase},
     {wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE},
     {wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE},
     {wxACCEL_SHIFT, WXK_DELETE, wxID_CUT}},
    m_data.menu().test(stc_data::MENU_DEBUG))
    .set(this);

  bind(this).menus(
    {{[=](wxCommandEvent& event) {
        Copy();
      },
      wxID_COPY},
     {[=](wxCommandEvent& event) {
        Cut();
      },
      wxID_CUT},
     {[=](wxCommandEvent& event) {
        Paste();
      },
      wxID_PASTE},
     {[=](wxCommandEvent& event) {
        Undo();
      },
      wxID_UNDO},
     {[=](wxCommandEvent& event) {
        Redo();
      },
      wxID_REDO},
     {[=](wxCommandEvent& event) {
        SelectAll();
      },
      wxID_SELECTALL},
     {[=](wxCommandEvent& event) {
        if (!GetReadOnly() && !is_hexmode())
          Clear();
      },
      wxID_DELETE},
     {[=](wxCommandEvent& event) {
        if (is_hexmode())
        {
          m_hexmode.goto_dialog();
        }
        else
        {
          if (static long val;
              (val = wxGetNumberFromUser(
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
        return true;
      },
      wxID_JUMP_TO},
     {[=](wxCommandEvent& event) {
        get_find_string();
        event.Skip();
      },
      wxID_FIND},
     {[=](wxCommandEvent& event) {
        get_find_string();
        event.Skip();
      },
      wxID_REPLACE},

     {[=](wxCommandEvent& event) {
        if (SelectionIsRectangle())
        {
          sort_selection(
            this,
            event.GetId() == wxID_SORT_ASCENDING ?
              string_sort_t() :
              string_sort_t().set(STRING_SORT_DESCENDING));
        }
        else if (const auto pos(wxGetNumberFromUser(
                   _("Input") + ":",
                   wxEmptyString,
                   _("Enter Sort Position"),
                   GetCurrentPos() + 1 - PositionFromLine(GetCurrentLine()),
                   1,
                   GetLineEndPosition(GetCurrentLine()),
                   this));
                 pos > 0)
        {
          sort_selection(
            this,
            event.GetId() == wxID_SORT_ASCENDING ?
              string_sort_t() :
              string_sort_t().set(STRING_SORT_DESCENDING),
            pos - 1);
        }
      },
      wxID_SORT_ASCENDING},

     {[=](wxCommandEvent& event) {
        m_frame->get_debug()->execute(
          event.GetId() - ID_EDIT_DEBUG_FIRST,
          this);
      },
      ID_EDIT_DEBUG_FIRST},

     {[=](wxCommandEvent& event) {
        switch (event.GetInt())
        {
          case stc_file::FILE_LOAD:
            if (
              get_lexer().scintilla_lexer().empty() &&
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
      },
      ID_EDIT_FILE_ACTION},

     {[=](wxCommandEvent& event) {
        browser_search(GetSelectedText().ToStdString());
      },
      idOpenWWW},
     {[=](wxCommandEvent& event) {
        link_open(link_t().set(LINK_OPEN));
      },
      idOpenLink},

     {[=](wxCommandEvent& event) {
        const std::string propnames(PropertyNames());
        const lexer_props l;

        std::string properties =
          (!propnames.empty() ? l.make_section("Current properties") :
                                std::string());

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

          for (tokenizer tkz(propnames, "\n"); tkz.has_more_tokens();)
          {
            const auto prop(tkz.get_next_token());
            properties +=
              l.make_key(prop, GetProperty(prop), DescribeProperty(prop));
          }
        }

        if (m_entry_dialog == nullptr)
        {
          m_entry_dialog = new stc_entry_dialog(
            properties,
            std::string(),
            window_data().size({300, 450}).button(wxOK).title(_("Properties")));
          m_entry_dialog->get_stc()->get_lexer().set(l);
        }
        else
        {
          m_entry_dialog->get_stc()->set_text(properties);
        }
        m_entry_dialog->Show();
      },
      idShowProperties},

     {[=](wxCommandEvent& event) {
        if (GetSelectedText().length() > 2)
          return;
        const wxString& caption = _("Enter Control Character");
        if (is_hexmode())
          return m_hexmode.control_char_dialog(caption);
        if (GetReadOnly())
        {
          if (GetSelectedText().length() == 1)
          {
            const char value = GetSelectedText().GetChar(0);
            wxMessageBox(
              wxString::Format("hex: %x dec: %d", value, value),
              _("Control Character"));
          }
          return;
        }

        static int value =
          ' '; // don't use 0 as default as nullptr is not handled
        if (GetSelectedText().length() == 1)
          value = GetSelectedText().GetChar(0);
        int new_value;
        if (
          (new_value = (int)wxGetNumberFromUser(
             _("Input") + " 0 - 255:",
             wxEmptyString,
             caption,
             value,
             0,
             255,
             this)) < 0)
          return;

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
      },
      ID_EDIT_CONTROL_CHAR},

     {[=](wxCommandEvent& event) {
        AnnotationSetText(GetCurrentLine(), event.GetString());
      },
      ID_EDIT_DEBUG_VARIABLE},

     {[=](wxCommandEvent& event) {
        if (CallTipActive())
          CallTipCancel();

        const auto pos = GetCurrentPos();

        if (is_hexmode())
        {
          CallTipShow(pos, m_hexmode.get_info());
          return;
        }

        const auto word =
          (!GetSelectedText().empty() ? GetSelectedText().ToStdString() :
                                        get_word_at_pos(pos));

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
            base10_ok  = (base10_val != 0);
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
            if (base10_ok && !base16_ok)
              stream << "hex: " << std::hex << base10_val;
            else if (!base10_ok && base16_ok)
              stream << "dec: " << base16_val;
            else if (base10_ok && base16_ok)
              stream << "dec: " << base16_val << " hex: " << std::hex
                     << base10_val;
          }
        }

        if (!stream.str().empty())
        {
          CallTipShow(pos, stream.str());
          clipboard_add(stream.str());
        }
      },
      idHexDecCalltip},

     {[=](wxCommandEvent& event) {
        MultiEdgeClearAll();
      },
      idEdgeClear},
     {[=](wxCommandEvent& event) {
        MultiEdgeAddLine(GetColumn(GetCurrentPos()), GetEdgeColour());
      },
      idEdgeSet},
     {[=](wxCommandEvent& event) {
        LowerCase();
      },
      idLowercase},
     {[=](wxCommandEvent& event) {
        UpperCase();
      },
      idUppercase},
     {[=](wxCommandEvent& event) {
        fold_all();
      },
      idfold_all},
     {[=](wxCommandEvent& event) {
        beautify().stc(*this);
      },
      idBeautify},
     {[=](wxCommandEvent& event) {
        for (int i = 0; i < GetLineCount(); i++)
          EnsureVisible(i);
      },
      idUnfold_all},
     {[=](wxCommandEvent& event) {
        m_data
          .flags(stc_data::window_t().set(stc_data::WIN_HEX), control_data::XOR)
          .inject();
      },
      idHex},

     {[=](wxCommandEvent& event) {
        config("blame.author").toggle(true);
      },
      m_id_margin_text_author},
     {[=](wxCommandEvent& event) {
        config("blame.date").toggle(true);
      },
      m_id_margin_text_date},
     {[=](wxCommandEvent& event) {
        config("blame.id").toggle(false);
      },
      m_id_margin_text_id},
     {[=](wxCommandEvent& event) {
        reset_margins(margin_t().set(MARGIN_TEXT));
        m_margin_text_click = -1;
        const item_vector& iv(m_config_items);
        SetWrapMode(iv.find<long>(_("stc.Wrap line")));
      },
      m_id_margin_text_hide},

     {[=](wxCommandEvent& event) {
        SetZoom(++m_zoom);
      },
      idZoomIn},
     {[=](wxCommandEvent& event) {
        SetZoom(--m_zoom);
      },
      idZoomOut},

     {[=](wxCommandEvent& event) {
        find_replace_data::get()->set_search_down(true);
        find_next();
      },
      ID_EDIT_FIND_NEXT},
     {[=](wxCommandEvent& event) {
        find_replace_data::get()->set_search_down(false);
        find_next();
      },
      ID_EDIT_FIND_PREVIOUS},
     {[=](wxCommandEvent& event) {
        link_open(link_t().set(LINK_OPEN_MIME));
      },
      idopen_mime},

     {[=](wxCommandEvent& event) {
        const auto level        = GetFoldLevel(GetCurrentLine());
        const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
                                    GetCurrentLine() :
                                    GetFoldParent(GetCurrentLine());
        ToggleFold(line_to_fold);
      },
      idToggleFold},

     {[=](wxCommandEvent& event) {
        vcs_execute(
          m_frame,
          event.GetId() - ID_EDIT_VCS_LOWEST - 1,
          std::vector<path>{get_filename().data()});
      },
      ID_EDIT_VCS_LOWEST},

     {[=](wxCommandEvent& event) {
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
            int eol_mode = wxSTC_EOL_LF; // default ID_EDIT_EOL_UNIX
            if (event.GetId() == ID_EDIT_EOL_DOS)
              eol_mode = wxSTC_EOL_CRLF;
            else if (event.GetId() == ID_EDIT_EOL_MAC)
              eol_mode = wxSTC_EOL_CR;

            ConvertEOLs(eol_mode);
            SetEOLMode(eol_mode);
            frame::update_statusbar(this, "PaneFileType");
          }
        }
      },
      ID_EDIT_EOL_DOS},

     {[=](wxCommandEvent& event) {
        auto line = MarkerNext(GetCurrentLine() + 1, 0xFFFF);
        if (line == -1)
        {
          line = MarkerNext(0, 0xFFFF);
        }
        if (line != -1)
        {
          GotoLine(line);
        }
        else
        {
          log::status(_("No markers present"));
        }
      },
      idMarkerNext},
     {[=](wxCommandEvent& event) {
        auto line = MarkerPrevious(GetCurrentLine() - 1, 0xFFFF);
        if (line == -1)
        {
          line = MarkerPrevious(GetLineCount() - 1, 0xFFFF);
        }
        if (line != -1)
        {
          GotoLine(line);
        }
        else
        {
          log::status(_("No markers present"));
        }
      },
      idMarkerPrevious}});

  bind_other();
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
    if (sel.empty() && link_open(link_t().set(LINK_OPEN_MIME).set(LINK_CHECK)))
    {
      menu.append({{}, {idopen_mime, _("&Preview")}});
    }
    else if (std::string filename;
             link_open(link_t().set(LINK_OPEN).set(LINK_CHECK), &filename))
    {
      menu.append({{}, {idOpenLink, _("Open") + " " + filename}});
    }
  }

  if (GetEdgeMode() == wxSTC_EDGE_MULTILINE)
  {
    menu.append({{},
                 {idEdgeSet, _("Edge Column")},
                 {idEdgeClear, _("Edge Column Reset")}});
  }

  if (m_data.menu().test(stc_data::MENU_OPEN_WWW) && !sel.empty())
  {
    menu.append({{}, {idOpenWWW, _("&Search")}});
  }

  if (
    m_data.menu().test(stc_data::MENU_DEBUG) &&
    matches_one_of(
      get_filename().extension(),
      m_frame->get_debug()->debug_entry().extensions()))
  {
    m_frame->get_debug()->add_menu(&menu, true);
  }

  if (
    m_data.menu().test(stc_data::MENU_VCS) && get_filename().file_exists() &&
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

  const bool beautify_add(beautify().is_active() && !beautify().is_auto());

  if (!GetReadOnly())
  {
    if (!sel.empty())
    {
      auto* menuSelection = new wex::menu(
        menu.style(),
        {{idUppercase, _("&Uppercase\tF11")},
         {idLowercase, _("&Lowercase\tF12")}});

      if (beautify_add)
      {
        menuSelection->append({{}, {idBeautify, _("&Beautify")}});
      }

      if (get_number_of_lines(sel) > 1)
      {
        menuSelection->append(
          {{},
           {new wex::menu(
              menu.style(),
              {{wxID_SORT_ASCENDING}, {wxID_SORT_DESCENDING}}),
            _("&Sort")}});
      }

      menu.append({{}, {menuSelection, _("&Selection")}});
    }
  }

  if (!GetReadOnly() && (CanUndo() || CanRedo()))
  {
    menu.append({{}});
    if (CanUndo())
      menu.append({{wxID_UNDO}});
    if (CanRedo())
      menu.append({{wxID_REDO}});
  }

  if (
    !GetReadOnly() && sel.empty() && beautify_add &&
    beautify().is_supported(m_lexer))
  {
    menu.append({{}, {idBeautify, _("&Beautify")}});
  }

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
    sel.empty() && GetProperty("fold") == "1" && m_lexer.is_ok() &&
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
  if (!config(_("stc.Show match")).get(true))
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
  if (const auto brace_match = BraceMatch(pos);
      brace_match != wxSTC_INVALID_POSITION)
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
  auto* menu = new wex::menu({{ID_EDIT_EOL_DOS, "&DOS", menu_item::CHECK},
                              {ID_EDIT_EOL_MAC, "&MAC", menu_item::CHECK},
                              {ID_EDIT_EOL_UNIX, "&UNIX", menu_item::CHECK},
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
