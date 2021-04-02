////////////////////////////////////////////////////////////////////////////////
// Name:      stc/bind.cpp
// Purpose:   Implementation of class wex::stc method bind_all
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <numeric>
#include <vector>
#include <wex/beautify.h>
#include <wex/bind.h>
#include <wex/config.h>
#include <wex/debug-entry.h>
#include <wex/defs.h>
#include <wex/frame.h>
#include <wex/frd.h>
#include <wex/item-vector.h>
#include <wex/lexer-props.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/menu.h>
#include <wex/path.h>
#include <wex/sort.h>
#include <wex/stc-bind.h>
#include <wex/stc-entry-dialog.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wx/accel.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>

namespace wex
{
  void edit_control_char(stc* stc)
  {
    if (stc->GetSelectedText().length() > 2)
      return;

    const wxString& caption = _("Enter Control Character");
    if (stc->is_hexmode())
      return stc->get_hexmode().control_char_dialog(caption);

    if (stc->GetReadOnly())
    {
      if (stc->GetSelectedText().length() == 1)
      {
        const char value = stc->GetSelectedText().GetChar(0);
        wxMessageBox(
          wxString::Format("hex: %x dec: %d", value, value),
          _("Control Character"));
      }
      return;
    }

    static int value = ' '; // don't use 0 as default as nullptr is not handled
    if (stc->GetSelectedText().length() == 1)
      value = stc->GetSelectedText().GetChar(0);
    int new_value;
    if (
      (new_value = (int)wxGetNumberFromUser(
         _("Input") + " 0 - 255:",
         wxEmptyString,
         caption,
         value,
         0,
         255,
         stc)) < 0)
      return;

    if (stc->GetSelectedText().length() == 1)
    {
      if (value != new_value)
      {
        stc->ReplaceSelection(std::to_string((char)new_value));
      }

      stc->SetSelection(stc->GetCurrentPos(), stc->GetCurrentPos() + 1);
    }
    else
    {
      char buffer[2];
      buffer[0] = (char)new_value;
      buffer[1] = 0;

      if (stc->get_vi().is_active())
      {
        stc->get_vi().command(std::string(buffer, 2));
      }
      else
      {
        stc->AddTextRaw(buffer, 1);
      }

      stc->process_char(new_value);
    }

    value = new_value;
  }

  void show_calltip(stc* stc)
  {
    if (stc->CallTipActive())
      stc->CallTipCancel();

    const auto pos = stc->GetCurrentPos();

    if (stc->is_hexmode())
    {
      stc->CallTipShow(pos, stc->get_hexmode().get_info());
      return;
    }

    const auto word =
      (!stc->GetSelectedText().empty() ? stc->GetSelectedText().ToStdString() :
                                         stc->get_word_at_pos(pos));

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
          stream << "dec: " << base16_val << " hex: " << std::hex << base10_val;
      }
    }

    if (!stream.str().empty())
    {
      stc->CallTipShow(pos, stream.str());
      clipboard_add(stream.str());
    }
  }
} // namespace wex

void wex::stc::bind_all()
{
  m_frame->bind_accelerators(
    this,
    {{wxACCEL_CTRL, 'D', id::stc::hex_dec_calltip},
     {wxACCEL_CTRL, 'K', ID_EDIT_CONTROL_CHAR},
     {wxACCEL_CTRL, 'Y', wxID_REDO},
     {wxACCEL_CTRL, 'Z', wxID_UNDO},
     {wxACCEL_CTRL, '=', id::stc::zoom_in},
     {wxACCEL_CTRL, '-', id::stc::zoom_out},
     {wxACCEL_CTRL, '0', id::stc::marker_previous},
     {wxACCEL_CTRL, '9', id::stc::marker_next},
     {wxACCEL_CTRL, WXK_INSERT, wxID_COPY},
     {wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT},
     {wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS},
     {wxACCEL_NORMAL, WXK_F7, wxID_SORT_ASCENDING},
     {wxACCEL_NORMAL, WXK_F8, wxID_SORT_DESCENDING},
     {wxACCEL_NORMAL, WXK_F9, id::stc::fold_all},
     {wxACCEL_NORMAL, WXK_F10, id::stc::unfold_all},
     {wxACCEL_NORMAL, WXK_F11, id::stc::uppercase},
     {wxACCEL_NORMAL, WXK_F12, id::stc::lowercase},
     {wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE},
     {wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE},
     {wxACCEL_SHIFT, WXK_DELETE, wxID_CUT}},
    m_data.menu().test(data::stc::MENU_DEBUG));

  bind(this).command(
    {{[=, this](wxCommandEvent& event) {
        Copy();
      },
      wxID_COPY},

     {[=, this](wxCommandEvent& event) {
        m_vi->command(event.GetString());
      },
      id::stc::vi_command},

     {[=, this](wxCommandEvent& event) {
        Cut();
      },
      wxID_CUT},

     {[=, this](wxCommandEvent& event) {
        Paste();
      },
      wxID_PASTE},

     {[=, this](wxCommandEvent& event) {
        Undo();
      },
      wxID_UNDO},

     {[=, this](wxCommandEvent& event) {
        Redo();
      },
      wxID_REDO},

     {[=, this](wxCommandEvent& event) {
        SelectAll();
      },
      wxID_SELECTALL},

     {[=, this](wxCommandEvent& event) {
        if (!GetReadOnly() && !is_hexmode())
          Clear();
      },
      wxID_DELETE},

     {[=, this](wxCommandEvent& event) {
        jump_action();
      },
      wxID_JUMP_TO},

     {[=, this](wxCommandEvent& event) {
        get_find_string();
        event.Skip();
      },
      wxID_FIND},

     {[=, this](wxCommandEvent& event) {
        get_find_string();
        event.Skip();
      },
      wxID_REPLACE},

     {[=, this](wxCommandEvent& event) {
        sort_action(event);
      },
      wxID_SORT_ASCENDING},

     {[=, this](wxCommandEvent& event) {
        m_frame->debug_exe(event.GetId() - ID_EDIT_DEBUG_FIRST, this);
      },
      ID_EDIT_DEBUG_FIRST},

     {[=, this](wxCommandEvent& event) {
        file_action(event);
      },
      ID_EDIT_FILE_ACTION},

     {[=, this](wxCommandEvent& event) {
        browser_search(GetSelectedText().ToStdString());
      },
      id::stc::open_www},

     {[=, this](wxCommandEvent& event) {
        link_open(link_t().set(LINK_OPEN));
      },
      id::stc::open_link},

     {[=, this](wxCommandEvent& event) {
        show_properties();
      },
      id::stc::show_properties},

     {[=, this](wxCommandEvent& event) {
        edit_control_char(this);
      },
      ID_EDIT_CONTROL_CHAR},

     {[=, this](wxCommandEvent& event) {
        AnnotationSetText(get_current_line(), event.GetString());
      },
      ID_EDIT_DEBUG_VARIABLE},

     {[=, this](wxCommandEvent& event) {
        show_calltip(this);
      },
      id::stc::hex_dec_calltip},

     {[=, this](wxCommandEvent& event) {
        MultiEdgeClearAll();
      },
      id::stc::edge_clear},

     {[=, this](wxCommandEvent& event) {
        MultiEdgeAddLine(GetColumn(GetCurrentPos()), GetEdgeColour());
      },
      id::stc::edge_set},

     {[=, this](wxCommandEvent& event) {
        LowerCase();
      },
      id::stc::lowercase},

     {[=, this](wxCommandEvent& event) {
        UpperCase();
      },
      id::stc::uppercase},
     {[=, this](wxCommandEvent& event) {
        fold_all();
      },
      id::stc::fold_all},

     {[=, this](wxCommandEvent& event) {
        beautify().stc(*this);
      },
      id::stc::beautify},

     {[=, this](wxCommandEvent& event) {
        for (int i = 0; i < get_line_count(); i++)
          EnsureVisible(i);
      },
      id::stc::unfold_all},

     {[=, this](wxCommandEvent& event) {
        m_data
          .flags(
            data::stc::window_t().set(data::stc::WIN_HEX),
            data::control::XOR)
          .inject();
      },
      id::stc::hex},

     {[=, this](wxCommandEvent& event) {
        config("blame.author").toggle(true);
      },
      id::stc::margin_text_author},

     {[=, this](wxCommandEvent& event) {
        config("blame.date").toggle(true);
      },
      id::stc::margin_text_date},

     {[=, this](wxCommandEvent& event) {
        config("blame.id").toggle(false);
      },
      id::stc::margin_text_id},

     {[=, this](wxCommandEvent& event) {
        reset_margins(margin_t().set(MARGIN_TEXT));
        m_margin_text_click = -1;
        const item_vector& iv(m_config_items);
        SetWrapMode(iv.find<long>(_("stc.Wrap line")));
      },
      id::stc::margin_text_hide},

     {[=, this](wxCommandEvent& event) {
        SetZoom(++m_zoom);
      },
      id::stc::zoom_in},

     {[=, this](wxCommandEvent& event) {
        SetZoom(--m_zoom);
      },
      id::stc::zoom_out},

     {[=, this](wxCommandEvent& event) {
        find_replace_data::get()->set_search_down(true);
        find_next();
      },
      ID_EDIT_FIND_NEXT},

     {[=, this](wxCommandEvent& event) {
        find_replace_data::get()->set_search_down(false);
        find_next();
      },
      ID_EDIT_FIND_PREVIOUS},

     {[=, this](wxCommandEvent& event) {
        link_open(link_t().set(LINK_OPEN_MIME));
      },
      id::stc::open_mime},

     {[=, this](wxCommandEvent& event) {
        const auto level        = GetFoldLevel(get_current_line());
        const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
                                    get_current_line() :
                                    GetFoldParent(get_current_line());
        ToggleFold(line_to_fold);
      },
      id::stc::toggle_fold},

     {[=, this](wxCommandEvent& event) {
        vcs_execute(
          m_frame,
          event.GetId() - ID_EDIT_VCS_LOWEST - 1,
          std::vector<path>{get_filename().data()});
      },
      ID_EDIT_VCS_LOWEST},

     {[=, this](wxCommandEvent& event) {
        eol_action(event);
      },
      id::stc::eol_dos},

     {[=, this](wxCommandEvent& event) {
        auto line = MarkerNext(get_current_line() + 1, 0xFFFF);
        if (line == -1)
        {
          line = MarkerNext(0, 0xFFFF);
        }
        if (line != -1)
        {
          goto_line(line);
        }
        else
        {
          log::status(_("No markers present"));
        }
      },
      id::stc::marker_next},

     {[=, this](wxCommandEvent& event) {
        auto line = MarkerPrevious(get_current_line() - 1, 0xFFFF);
        if (line == -1)
        {
          line = MarkerPrevious(get_line_count() - 1, 0xFFFF);
        }
        if (line != -1)
        {
          goto_line(line);
        }
        else
        {
          log::status(_("No markers present"));
        }
      },
      id::stc::marker_previous}});

  bind_other();
}

void wex::stc::build_popup_menu(menu& menu)
{
  const auto sel(GetSelectedText().ToStdString());

  if (get_current_line() == 0 && !lexers::get()->get_lexers().empty())
  {
    menu.append({{id::stc::show_properties, _("Properties")}});
  }

  if (m_data.menu().test(data::stc::MENU_OPEN_LINK))
  {
    if (sel.empty() && link_open(link_t().set(LINK_OPEN_MIME).set(LINK_CHECK)))
    {
      menu.append({{}, {id::stc::open_mime, _("&Preview")}});
    }
    else if (std::string filename;
             link_open(link_t().set(LINK_OPEN).set(LINK_CHECK), &filename))
    {
      menu.append({{}, {id::stc::open_link, _("Open") + " " + filename}});
    }
  }

  if (GetEdgeMode() == wxSTC_EDGE_MULTILINE)
  {
    menu.append(
      {{},
       {id::stc::edge_set, _("Edge Column")},
       {id::stc::edge_clear, _("Edge Column Reset")}});
  }

  if (m_data.menu().test(data::stc::MENU_OPEN_WWW) && !sel.empty())
  {
    menu.append({{}, {id::stc::open_www, _("&Search")}});
  }

  if (
    m_data.menu().test(data::stc::MENU_DEBUG) &&
    matches_one_of(
      get_filename().extension(),
      m_frame->debug_entry()->extensions()))
  {
    m_frame->debug_add_menu(menu, true);
  }

  if (
    m_data.menu().test(data::stc::MENU_VCS) && get_filename().file_exists() &&
    vcs::dir_exists(get_filename()))
  {
    menu.append({{}, {get_filename(), m_frame}});
  }

  if (!get_vi().is_active() && GetTextLength() > 0)
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
        {{id::stc::uppercase, _("&Uppercase\tF11")},
         {id::stc::lowercase, _("&Lowercase\tF12")}});

      if (beautify_add)
      {
        menuSelection->append({{}, {id::stc::beautify, _("&Beautify")}});
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
    beautify().is_supported(get_lexer()))
  {
    menu.append({{}, {id::stc::beautify, _("&Beautify")}});
  }

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
    sel.empty() && GetProperty("fold") == "1" && get_lexer().is_ok() &&
    !get_lexer().scintilla_lexer().empty())
  {
    menu.append(
      {{},
       {id::stc::toggle_fold, _("&Toggle Fold\tCtrl+T")},
       {id::stc::fold_all, _("&Fold All Lines\tF9")},
       {id::stc::unfold_all, _("&Unfold All Lines\tF10")}});
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

void wex::stc::eol_action(const wxCommandEvent& event)
{
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
      int eol_mode = wxSTC_EOL_LF; // default id::stc::eol_unix
      if (event.GetId() == id::stc::eol_dos)
        eol_mode = wxSTC_EOL_CRLF;
      else if (event.GetId() == id::stc::eol_mac)
        eol_mode = wxSTC_EOL_CR;

      ConvertEOLs(eol_mode);
      SetEOLMode(eol_mode);
      m_frame->update_statusbar(this, "PaneFileType");
    }
  }
}

void wex::stc::file_action(const wxCommandEvent& event)
{
  switch (event.GetInt())
  {
    case stc_file::FILE_LOAD:
      if (
        get_lexer().scintilla_lexer().empty() &&
        GetLength() < config("stc.max.Size lexer").get(10000000))
      {
        get_lexer().set(get_filename().lexer());
        config_get();
      }

      guess_type_and_modeline();
      log::status(_("Opened")) << get_filename();
      log::info("opened") << get_filename();
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
      log::info("saved") << get_filename();
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
}

void wex::stc::filetype_menu()
{
  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  auto* menu = new wex::menu(
    {{id::stc::eol_dos, "&DOS", menu_item::CHECK},
     {id::stc::eol_mac, "&Mac", menu_item::CHECK},
     {id::stc::eol_unix, "&Unix", menu_item::CHECK},
     {}});

  auto* hex = menu->AppendCheckItem(id::stc::hex, "&Hex");

  menu->FindItemByPosition(GetEOLMode())->Check();

  if (is_hexmode())
  {
    hex->Check();
  }

  PopupMenu(menu);

  delete menu;
}

void wex::stc::jump_action()
{
  if (is_hexmode())
  {
    m_hexmode.goto_dialog();
  }
  else if (static long val;
           (val = wxGetNumberFromUser(
              _("Input") + " 1 - " + std::to_string(get_line_count()) + ":",
              wxEmptyString,
              _("Enter Line Number"),
              m_data.control().line(), // initial value
              1,
              get_line_count(),
              this)) > 0)
  {
    m_data.control().line(val);
    data::stc(data::control().line(val), this).inject();
  }
}

void wex::stc::show_properties()
{
  const std::string propnames(PropertyNames());
  const lexer_props l;

  std::string properties =
    (!propnames.empty() ? l.make_section("Current properties") :
                          std::string()) +
    // Add current (global and lexer) properties.
    std::accumulate(
      lexers::get()->properties().begin(),
      lexers::get()->properties().end(),
      std::string(),
      [this, l](const std::string& a, const property& b) {
        return a + l.make_key(b.name(), GetProperty(b.name()));
      }) +
    std::accumulate(
      get_lexer().properties().begin(),
      get_lexer().properties().end(),
      std::string(),
      [this, l](const std::string& a, const property& b) {
        return a + l.make_key(b.name(), GetProperty(b.name()));
      });

  // Add available properties.
  if (!propnames.empty())
  {
    properties += "\n" + l.make_section("Available properties");

    for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
           propnames,
           boost::char_separator<char>("\n")))
    {
      properties += l.make_key(it, GetProperty(it), DescribeProperty(it));
    }
  }

  if (m_prop_dialog == nullptr)
  {
    m_prop_dialog = new stc_entry_dialog(
      properties,
      std::string(),
      data::window().size({300, 450}).button(wxOK).title(_("Properties")));
    m_prop_dialog->get_stc()->get_lexer().set(l);
  }
  else
  {
    m_prop_dialog->get_stc()->set_text(properties);
  }

  m_prop_dialog->Show();
}

void wex::stc::sort_action(const wxCommandEvent& event)
{
  if (SelectionIsRectangle())
  {
    sort(
      event.GetId() == wxID_SORT_ASCENDING ?
        sort::sort_t() :
        sort::sort_t().set(sort::SORT_DESCENDING))
      .selection(this);
  }
  else if (const auto pos(wxGetNumberFromUser(
             _("Input") + ":",
             wxEmptyString,
             _("Enter Sort Position"),
             GetCurrentPos() + 1 - PositionFromLine(get_current_line()),
             1,
             GetLineEndPosition(get_current_line()),
             this));
           pos > 0)
  {
    sort(
      event.GetId() == wxID_SORT_ASCENDING ?
        sort::sort_t() :
        sort::sort_t().set(sort::SORT_DESCENDING),
      pos - 1)
      .selection(this);
  }
}
