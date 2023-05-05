////////////////////////////////////////////////////////////////////////////////
// Name:      stc/bind.cpp
// Purpose:   Implementation of class wex::stc method bind_all
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/bind.h>
#include <wex/factory/defs.h>
#include <wex/factory/sort.h>
#include <wex/stc/beautify.h>
#include <wex/stc/bind.h>
#include <wex/stc/entry-dialog.h>
#include <wex/stc/stc.h>
#include <wex/syntax/lexer-props.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/path-lexer.h>
#include <wex/ui/debug-entry.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/item-vector.h>
#include <wex/ui/menu.h>
#include <wx/accel.h>
#include <wx/msgdlg.h>
#include <wx/numdlg.h>

#include <charconv>
#include <numeric>

namespace wex
{
void edit_control_char(stc* stc)
{
  if (stc->GetSelectedText().length() > 2)
    return;

  const std::string& caption = _("Enter Control Character");
  if (stc->is_hexmode())
    return stc->get_hexmode().control_char_dialog(caption);

  if (stc->GetReadOnly())
  {
    if (stc->GetSelectedText().length() == 1)
    {
      const char        value = stc->GetSelectedText().GetChar(0);
      std::stringstream stream;
      stream << "hex: " << std::hex << value << " dec: " << value;
      wxMessageBox(stream.str(), _("Control Character"));
    }
    return;
  }

  static int value = ' '; // don't use 0 as default as nullptr is not handled
  if (stc->GetSelectedText().length() == 1)
    value = stc->GetSelectedText().GetChar(0);
  int new_value;
  if (
    (new_value = static_cast<int>(wxGetNumberFromUser(
       _("Input") + " 0 - 255:",
       wxEmptyString,
       caption,
       value,
       0,
       255,
       stc))) < 0)
    return;

  if (stc->GetSelectedText().length() == 1)
  {
    if (value != new_value)
    {
      stc->ReplaceSelection(std::to_string(static_cast<char>(new_value)));
    }

    stc->SetSelection(stc->GetCurrentPos(), stc->GetCurrentPos() + 1);
  }
  else
  {
    char buffer[2];
    buffer[0] = static_cast<char>(new_value);
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

const std::string get_properties(
  const lexer_props&           l,
  const std::vector<property>& props,
  wxStyledTextCtrl*            stc)
{
  return std::accumulate(
    props.begin(),
    props.end(),
    std::string(),
    [stc, l](const std::string& a, const property& b)
    {
      return a + l.make_key(b.name(), stc->GetProperty(b.name()));
    });
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

  bind_wx();

  bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        m_vi->command(event.GetString());
      },
      id::stc::vi_command},

     {[=, this](wxCommandEvent& event)
      {
        jump_action();
      },
      wxID_JUMP_TO},

     {[=, this](wxCommandEvent& event)
      {
        get_find_string();
        event.Skip();
      },
      wxID_FIND},

     {[=, this](wxCommandEvent& event)
      {
        get_find_string();
        event.Skip();
      },
      wxID_REPLACE},

     {[=, this](const wxCommandEvent& event)
      {
        sort_action(event);
      },
      wxID_SORT_ASCENDING},

     {[=, this](wxCommandEvent& event)
      {
        m_frame->debug_exe(event.GetId() - ID_EDIT_DEBUG_FIRST, this);
      },
      ID_EDIT_DEBUG_FIRST},

     {[=, this](const wxCommandEvent& event)
      {
        file_action(event);
      },
      ID_EDIT_FILE_ACTION},

     {[=, this](wxCommandEvent& event)
      {
        browser_search(GetSelectedText().ToStdString());
      },
      id::stc::open_www},

     {[=, this](wxCommandEvent& event)
      {
        link_open(link_t().set(LINK_OPEN));
      },
      id::stc::open_link},

     {[=, this](wxCommandEvent& event)
      {
        show_properties();
      },
      id::stc::show_properties},

     {[=, this](wxCommandEvent& event)
      {
        edit_control_char(this);
      },
      ID_EDIT_CONTROL_CHAR},

     {[=, this](wxCommandEvent& event)
      {
        AnnotationSetText(get_current_line(), event.GetString());
      },
      ID_EDIT_DEBUG_VARIABLE},

     {[=, this](wxCommandEvent& event)
      {
        show_ascii_value();
      },
      id::stc::hex_dec_calltip},

     {[=, this](wxCommandEvent& event)
      {
        MultiEdgeClearAll();
      },
      id::stc::edge_clear},

     {[=, this](wxCommandEvent& event)
      {
        MultiEdgeAddLine(GetColumn(GetCurrentPos()), GetEdgeColour());
      },
      id::stc::edge_set},

     {[=, this](wxCommandEvent& event)
      {
        LowerCase();
      },
      id::stc::lowercase},

     {[=, this](wxCommandEvent& event)
      {
        UpperCase();
      },
      id::stc::uppercase},
     {[=, this](wxCommandEvent& event)
      {
        fold(true);
      },
      id::stc::fold_all},

     {[=, this](wxCommandEvent& event)
      {
        beautify().stc(*this);
      },
      id::stc::beautify},

     {[=, this](wxCommandEvent& event)
      {
        for (int i = 0; i < get_line_count(); i++)
          EnsureVisible(i);
      },
      id::stc::unfold_all},

     {[=, this](wxCommandEvent& event)
      {
        m_data
          .flags(
            data::stc::window_t().set(data::stc::WIN_HEX),
            data::control::XOR)
          .inject();
      },
      id::stc::hex},

     {[=, this](wxCommandEvent& event)
      {
        config("blame.author").toggle(true);
        m_frame->vcs_blame(this);
      },
      id::stc::margin_text_author},

     {[=, this](wxCommandEvent& event)
      {
        blame_revision();
      },
      id::stc::margin_text_blame_revision},

     {[=, this](wxCommandEvent& event)
      {
        blame_revision("~1");
      },
      id::stc::margin_text_blame_revision_previous},

     {[=, this](wxCommandEvent& event)
      {
        config("blame.date").toggle(true);
        m_frame->vcs_blame(this);
      },
      id::stc::margin_text_date},

     {[=, this](wxCommandEvent& event)
      {
        config("blame.id").toggle(false);
        m_frame->vcs_blame(this);
      },
      id::stc::margin_text_id},

     {[=, this](wxCommandEvent& event)
      {
        reset_margins(margin_t().set(MARGIN_TEXT));
        m_margin_text_click = -1;
        const item_vector iv(m_config_items);
        SetWrapMode(iv.find<long>(_("stc.Wrap line")));
      },
      id::stc::margin_text_hide},

     {[=, this](wxCommandEvent& event)
      {
        SetZoom(++m_zoom);
      },
      id::stc::zoom_in},

     {[=, this](wxCommandEvent& event)
      {
        SetZoom(--m_zoom);
      },
      id::stc::zoom_out},

     {[=, this](wxCommandEvent& event)
      {
        find_replace_data::get()->set_search_down(true);
        find_next();
      },
      ID_EDIT_FIND_NEXT},

     {[=, this](wxCommandEvent& event)
      {
        find_replace_data::get()->set_search_down(false);
        find_next();
      },
      ID_EDIT_FIND_PREVIOUS},

     {[=, this](wxCommandEvent& event)
      {
        link_open(link_t().set(LINK_OPEN_MIME));
      },
      id::stc::open_mime},

     {[=, this](wxCommandEvent& event)
      {
        const auto level        = GetFoldLevel(get_current_line());
        const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
                                    get_current_line() :
                                    GetFoldParent(get_current_line());
        ToggleFold(line_to_fold);
      },
      id::stc::toggle_fold},

     {[=, this](wxCommandEvent& event)
      {
        m_renamed.clear();

        m_frame->vcs_execute(
          event.GetId() - ID_EDIT_VCS_LOWEST - 1,
          std::vector<wex::path>{path().data()});
      },
      ID_EDIT_VCS_LOWEST},

     {[=, this](wxCommandEvent& event)
      {
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

     {[=, this](wxCommandEvent& event)
      {
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

  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        eol_action(event);
      },
      id::stc::eol_dos,
      id::stc::eol_mac}});

  bind_other();
}

void wex::stc::build_popup_menu(menu& menu)
{
  if (get_current_line() == 0 && !lexers::get()->get_lexers().empty())
  {
    menu.append({{id::stc::show_properties, _("Properties")}});
  }

  if (GetEdgeMode() == wxSTC_EDGE_MULTILINE)
  {
    menu.append(
      {{},
       {id::stc::edge_set, _("Edge Column")},
       {id::stc::edge_clear, _("Edge Column Reset")}});
  }

  build_popup_menu_link(menu);

  if (
    m_data.menu().test(data::stc::MENU_DEBUG) &&
    matches_one_of(path().extension(), m_frame->debug_entry()->extensions()))
  {
    m_frame->debug_add_menu(menu, true);
  }

  if (
    m_data.menu().test(data::stc::MENU_VCS) && path().file_exists() &&
    m_frame->vcs_dir_exists(path()))
  {
    menu.append({{}, {path(), m_frame}});
  }

  if (!get_vi().is_active() && GetTextLength() > 0)
  {
    menu.append({{}, {wxID_FIND}});
  }

  build_popup_menu_edit(menu);

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
    GetSelectedText().ToStdString().empty() && GetProperty("fold") == "1" &&
    get_lexer().is_ok() && !get_lexer().scintilla_lexer().empty())
  {
    menu.append(
      {{},
       {id::stc::toggle_fold, _("&Toggle Fold\tCtrl+T")},
       {id::stc::fold_all, _("&Fold All Lines\tF9")},
       {id::stc::unfold_all, _("&Unfold All Lines\tF10")}});
  }
}

void wex::stc::build_popup_menu_edit(menu& menu)
{
  menu.append({{}, {menu_item::EDIT}});

  if (GetReadOnly())
  {
    return;
  }

  const bool beautify_add(beautify().is_active() && !beautify().is_auto());
  const auto sel(GetSelectedText().ToStdString());

  if (!get_vi().is_active() && GetTextLength() > 0)
  {
    menu.append({{wxID_REPLACE}});
  }

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

  if (CanUndo() || CanRedo())
  {
    menu.append({{}});

    if (CanUndo())
    {
      menu.append({{wxID_UNDO}});
    }

    if (CanRedo())
    {
      menu.append({{wxID_REDO}});
    }
  }

  if (sel.empty() && beautify_add && beautify().is_supported(get_lexer()))
  {
    menu.append({{}, {id::stc::beautify, _("&Beautify")}});
  }
}

void wex::stc::build_popup_menu_link(menu& menu)
{
  const auto sel(GetSelectedText().ToStdString());

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

  if (m_data.menu().test(data::stc::MENU_OPEN_WWW) && !sel.empty())
  {
    menu.append({{}, {id::stc::open_www, _("&Search")}});
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
        auto l(path_lexer(path()).lexer());

        // If not in visual mode, inform the rfw lexer.
        if (l.scintilla_lexer() == "rfw" && !is_visual())
        {
          l.add_keywords("EX", 1);
        }

        get_lexer().set(l);
        config_get();
      }

      guess_type_and_modeline();
      log::status(_("Opened")) << path();
      log::info("opened") << path();
      fold();

      // This is to take care that current dir follows page selection.
      // Which is convenient for git grep, ls etc. and opening from stc window.
      path::current(path().data().parent_path());

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
      get_lexer().set(path_lexer(path()).lexer());
      SetName(path().string());
      [[fallthrough]];

    case stc_file::FILE_SAVE:
      SetReadOnly(path().is_readonly());
      marker_delete_all_change();
      log::status(_("Saved")) << path();
      log::info("saved") << path();
      break;
  }

  if (path_lexer(path()).lexer().language() == "xml")
  {
    if (const pugi::xml_parse_result result =
          pugi::xml_document().load_file(path().string().c_str());
        !result)
    {
      xml_error(path(), &result, this);
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

void wex::stc::show_ascii_value()
{
  if (CallTipActive())
    CallTipCancel();

  const auto pos = GetCurrentPos();

  if (is_hexmode())
  {
    CallTipShow(pos, get_hexmode().get_info());
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
    bool base10_ok = false, base16_ok = false;

    if (
      std::from_chars(word.data(), word.data() + word.size(), base10_val).ec ==
      std::errc())
    {
      base10_ok = true;
    }

    if (
      std::from_chars(word.data(), word.data() + word.size(), base16_val, 16)
        .ec == std::errc())
    {
      base16_ok = true;
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
    CallTipShow(pos, stream.str());
    clipboard_add(stream.str());
  }
}

void wex::stc::show_properties()
{
  const std::string propnames(PropertyNames());
  const lexer_props l;
  auto properties = (!propnames.empty() ? l.make_section("Current properties") :
                                          std::string()) +
                    // Add current (global and lexer) properties.
                    get_properties(l, lexers::get()->properties(), this) +
                    get_properties(l, get_lexer().properties(), this);

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
    m_prop_dialog->get_stc()->get_vi().use(ex::VISUAL);
  }
  else
  {
    m_prop_dialog->get_stc()->get_vi().mode().command();
    m_prop_dialog->get_stc()->set_text(properties);
  }

  m_prop_dialog->Show();
}

void wex::stc::sort_action(const wxCommandEvent& event)
{
  if (SelectionIsRectangle())
  {
    factory::sort(
      event.GetId() == wxID_SORT_ASCENDING ?
        factory::sort::sort_t() :
        factory::sort::sort_t().set(factory::sort::SORT_DESCENDING))
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
    factory::sort(
      event.GetId() == wxID_SORT_ASCENDING ?
        factory::sort::sort_t() :
        factory::sort::sort_t().set(factory::sort::SORT_DESCENDING),
      pos - 1)
      .selection(this);
  }
}
