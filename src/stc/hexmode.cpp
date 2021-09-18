////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wex::hexmode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/factory/stc.h>
#include <wex/hexmode.h>
#include <wex/item-dialog.h>
#include <wex/item.h>
#include <wex/lexers.h>
#include <wx/numdlg.h>
#include <wx/spinctrl.h>

#include "hexmode-line.h"

import<regex>;

namespace wex
{
int get_hex_number(const std::string& caption, factory::stc* stc, int value)
{
  const std::string message(_("Input") + " 00 - FF");
  const data::item  data(wex::data::item(
    data::item().min(0).max(255).window(
      data::window().title(caption).parent(stc)),
    value));

  item::use_config(false);
  item_dialog dlg({{message, item::SPINCTRL, data}}, data.window());
  item::use_config(true);

  (reinterpret_cast<wxSpinCtrl*>(dlg.find(message).window()))->SetBase(16);

  return dlg.ShowModal() == wxID_CANCEL ?
           -1 :
           std::any_cast<int>(dlg.get_item_value(message));
}

} // namespace wex

wex::hexmode::hexmode(factory::stc* stc, size_t bytes_per_line)
  : factory::hexmode(stc, bytes_per_line)
{
}

void wex::hexmode::activate()
{
  factory::hexmode::activate();

  get_stc()->SetControlCharSymbol('.');
  get_stc()->SetEdgeMode(wxSTC_EDGE_NONE);
  get_stc()->SetViewEOL(false);
  get_stc()->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  if (get_stc()->is_visual())
  {
    get_stc()->BeginUndoAction();
    set_text(get_stc()->get_text());
  }

  lexers::get()->apply(get_stc());
}

void wex::hexmode::append_text(const std::string& buffer)
{
  if (is_active())
  {
    m_buffer += buffer;
    m_buffer_original = m_buffer;
    get_stc()->append_text(lines(buffer));
  }
}

void wex::hexmode::control_char_dialog(const std::string& caption)
{
  if (hexmode_line ml(this, get_stc()->GetSelectionStart());
      ml.is_ascii_field() && get_stc()->GetSelectedText().size() == 1)
  {
    if (const auto new_value(get_hex_number(
          caption,
          get_stc(),
          static_cast<int>(get_stc()->GetSelectedText().GetChar(0))));
        new_value >= 0)
    {
      ml.replace(new_value);
    }
  }
  else if (ml.is_hex_field() && get_stc()->GetSelectedText().size() == 2)
  {
    if (long value; get_stc()->GetSelectedText().ToLong(&value, 16))
    {
      if (const auto new_value(
            get_hex_number(caption, get_stc(), static_cast<int>(value)));
          new_value >= 0)
      {
        ml.replace_hex(new_value);
      }
    }
  }
}

void wex::hexmode::deactivate()
{
  factory::hexmode::deactivate();

  get_stc()->SetControlCharSymbol(0);
  get_stc()->SetEdgeMode(config(_("stc.Edge line")).get(wxSTC_EDGE_NONE));
  get_stc()->SetViewEOL(config(_("stc.End of line")).get(false));
  get_stc()->SetViewWhiteSpace(
    config(_("stc.Whitespace visible")).get(wxSTC_WS_INVISIBLE));

  if (get_stc()->is_visual())
  {
    get_stc()->EndUndoAction();
    get_stc()->clear(false);
    get_stc()->append_text(m_buffer);
  }

  get_stc()->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
}

bool wex::hexmode::erase(int count, int pos)
{
  return pos == wxSTC_INVALID_POSITION ? hexmode_line(this).erase(count) :
                                         hexmode_line(this, pos).erase(count);
}

const std::string wex::hexmode::get_info()
{
  return hexmode_line(this).info();
}

bool wex::hexmode::goto_dialog()
{
  if (const auto val(wxGetNumberFromUser(
        _("Input") + " 0 - " + std::to_string(m_buffer.size() - 1) + ":",
        wxEmptyString,
        _("Enter Byte Offset"),
        m_goto, // initial value
        0,
        m_buffer.size() - 1,
        get_stc()));
      val < 0)
  {
    return false;
  }
  else
  {
    m_goto = val;
    return hexmode_line(this, val, false).set_pos();
  }
}

bool wex::hexmode::highlight_other()
{
  if (const auto pos = get_stc()->GetCurrentPos(); highlight_other(pos))
  {
    return true;
  }
  else if (get_stc()->PositionFromLine(pos) != pos)
  {
    return highlight_other(pos - 1);
  }

  return false;
}

bool wex::hexmode::highlight_other(int pos)
{
  if (const auto brace_match = hexmode_line(this, pos).other_field();
      brace_match != wxSTC_INVALID_POSITION)
  {
    get_stc()->BraceHighlight(
      pos,
      get_stc()->PositionFromLine(get_stc()->LineFromPosition(pos)) +
        brace_match);
    return true;
  }
  else
  {
    get_stc()->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
    return false;
  }
}

bool wex::hexmode::insert(const std::string& text, int pos)
{
  return pos == wxSTC_INVALID_POSITION ? hexmode_line(this).insert(text) :
                                         hexmode_line(this, pos).insert(text);
}

bool wex::hexmode::replace(char c, int pos)
{
  return pos == wxSTC_INVALID_POSITION ? hexmode_line(this).replace(c) :
                                         hexmode_line(this, pos).replace(c);
}

bool wex::hexmode::replace_target(const std::string& replacement, bool settext)
{
  if (
    get_stc()->GetTargetStart() == wxSTC_INVALID_POSITION ||
    get_stc()->GetTargetEnd() == wxSTC_INVALID_POSITION ||
    get_stc()->GetTargetEnd() <= get_stc()->GetTargetStart() ||
    (replacement.size() % 2) > 0 ||
    !std::regex_match(replacement, std::regex("[0-9A-F]*")))
  {
    return false;
  }

  // If we have:
  // 30 31 32 33 34 35
  // RT: 31 32 -> 39
  //     30 39 33 34 35 (delete)
  // RT: 31 32 -> 39 39
  //     30 39 39 33 34 35 (replace)
  // RT: 31 32 -> 39 39 39
  //     30 39 39 39 33 34 35 (insert)
  auto start = get_stc()->GetTargetStart();

  for (int i = 0; i < static_cast<int>(replacement.size());
       i = i + 2, start = start + each_hex_field())
  {
    // replace
    if (start <= get_stc()->GetTargetEnd())
    {
      hexmode_line(this, start)
        .replace({replacement[i], replacement[i + 1]}, settext);
    }
    // insert
    else
    {
      hexmode_line(this, start).insert({replacement[i], replacement[i + 1]});
    }
  }

  // delete
  if (start < get_stc()->GetTargetEnd())
  {
    hexmode_line(this, start)
      .erase((get_stc()->GetTargetEnd() - start) / each_hex_field(), settext);
  }

  get_stc()->SetTargetEnd(
    get_stc()->GetTargetStart() + replacement.size() * each_hex_field());

  return true;
}

bool wex::hexmode::set(bool on, bool use_modification_markers)
{
  if (is_active() == on)
    return false;

  get_stc()->use_modification_markers(false);

  const bool modified = (get_stc()->GetModify());

  on ? activate() : deactivate();

  if (!modified)
  {
    get_stc()->EmptyUndoBuffer();
    get_stc()->SetSavePoint();
  }

  get_stc()->use_modification_markers(use_modification_markers);

  return true;
}

void wex::hexmode::set_pos(const wxKeyEvent& event)
{
  hexmode_line(this).set_pos(event);
}

void wex::hexmode::set_text(const std::string text)
{
  if (!is_active())
    return;

  m_buffer.clear();
  m_buffer_original.clear();

  get_stc()->SelectNone();
  get_stc()->position_save();
  get_stc()->clear(false);

  append_text(text);

  get_stc()->position_restore();
}

bool wex::hexmode::sync()
{
  if (!is_active())
  {
    return false;
  }

  set_text(m_buffer);

  return true;
}

void wex::hexmode::undo()
{
  if (is_active())
  {
    m_buffer = m_buffer_original;
  }

  // For hex mode the first min_size bytes should be hex fields (or space).
  const int min_size = bytes_per_line() * each_hex_field();

  make_active(
    (get_stc()->GetTextLength() > min_size &&
     std::regex_match(
       get_stc()->GetTextRange(0, min_size).ToStdString(),
       std::regex("([0-9A-F][0-9A-F] )+ *"))));
}
