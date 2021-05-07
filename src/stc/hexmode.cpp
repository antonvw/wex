////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wex::hexmode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "hexmode-line.h"
#include <wex/config.h>
#include <wex/hexmode.h>
#include <wex/item-dialog.h>
#include <wex/item.h>
#include <wex/lexers.h>
#include <wex/stc.h>
#include <wx/numdlg.h>
#include <wx/spinctrl.h>

namespace wex
{
int get_hex_number(const std::string& caption, wex::stc* stc, int value)
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

wex::hexmode::hexmode(wex::stc* stc, size_t bytesPerLine)
  : m_stc(stc)
  , m_bytes_per_line(bytesPerLine)
  , m_each_hex_field(3)
{
}

void wex::hexmode::activate()
{
  m_active = true;

  m_stc->SetControlCharSymbol('.');
  m_stc->SetEdgeMode(wxSTC_EDGE_NONE);
  m_stc->SetViewEOL(false);
  m_stc->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  m_stc->BeginUndoAction();

  set_text(m_stc->get_text());

  lexers::get()->apply(m_stc);
}

void wex::hexmode::append_text(const std::string& buffer)
{
  if (!m_active)
    return;

  m_buffer += buffer;
  m_buffer_original = m_buffer;

  std::string text;

  text.reserve(
    // number of lines
    m_stc->eol().size() * (buffer.size() / m_bytes_per_line) +
    // hex field
    m_each_hex_field * buffer.size() +
    // ascii field
    buffer.size());

  for (size_t offset = 0; offset < buffer.size(); offset += m_bytes_per_line)
  {
    text += make_line(buffer, offset);
  }

  m_stc->append_text(text);
}

void wex::hexmode::control_char_dialog(const std::string& caption)
{
  if (hexmode_line ml(this, m_stc->GetSelectionStart());
      ml.is_ascii_field() && m_stc->GetSelectedText().size() == 1)
  {
    if (const auto new_value(get_hex_number(
          caption,
          m_stc,
          static_cast<int>(m_stc->GetSelectedText().GetChar(0))));
        new_value >= 0)
    {
      ml.replace(new_value);
    }
  }
  else if (ml.is_hex_field() && m_stc->GetSelectedText().size() == 2)
  {
    if (long value; m_stc->GetSelectedText().ToLong(&value, 16))
    {
      if (const auto new_value(
            get_hex_number(caption, m_stc, static_cast<int>(value)));
          new_value >= 0)
      {
        ml.replace_hex(new_value);
      }
    }
  }
}

void wex::hexmode::deactivate()
{
  m_active = false;

  m_stc->EndUndoAction();
  m_stc->SetControlCharSymbol(0);
  m_stc->SetEdgeMode(config(_("stc.Edge line")).get(wxSTC_EDGE_NONE));
  m_stc->SetViewEOL(config(_("stc.End of line")).get(false));
  m_stc->SetViewWhiteSpace(
    config(_("stc.Whitespace visible")).get(wxSTC_WS_INVISIBLE));
  m_stc->clear(false);
  m_stc->append_text(m_buffer);
  m_stc->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
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
        m_stc));
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
  if (const auto pos = m_stc->GetCurrentPos(); highlight_other(pos))
  {
    return true;
  }
  else if (m_stc->PositionFromLine(pos) != pos)
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
    m_stc->BraceHighlight(
      pos,
      m_stc->PositionFromLine(m_stc->LineFromPosition(pos)) + brace_match);
    return true;
  }
  else
  {
    m_stc->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
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

const std::string
wex::hexmode::make_line(const std::string& buffer, size_t offset) const
{
  std::string field_hex, field_ascii;

  auto count = buffer.size() - offset;
  count      = (m_bytes_per_line < count ? m_bytes_per_line : count);

  for (size_t byte = 0; byte < count; byte++)
  {
    const unsigned char c = buffer[offset + byte];

    char buff[4];
    snprintf(buff, sizeof(buff), "%02X ", c);
    field_hex += buff;
    field_ascii += printable(c, m_stc);
  }

  const auto field_spaces =
    std::string((m_bytes_per_line - count) * m_each_hex_field, ' ');

  return field_hex + field_spaces + field_ascii +
         (buffer.size() - offset > m_bytes_per_line ? m_stc->eol() :
                                                      std::string());
}

bool wex::hexmode::replace_target(const std::string& replacement, bool settext)
{
  if (
    m_stc->GetTargetStart() == wxSTC_INVALID_POSITION ||
    m_stc->GetTargetEnd() == wxSTC_INVALID_POSITION ||
    m_stc->GetTargetEnd() <= m_stc->GetTargetStart() ||
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
  auto start = m_stc->GetTargetStart();

  for (int i = 0; i < static_cast<int>(replacement.size());
       i = i + 2, start = start + m_each_hex_field)
  {
    // replace
    if (start <= m_stc->GetTargetEnd())
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
  if (start < m_stc->GetTargetEnd())
  {
    hexmode_line(this, start)
      .erase((m_stc->GetTargetEnd() - start) / m_each_hex_field, settext);
  }

  m_stc->SetTargetEnd(
    m_stc->GetTargetStart() + replacement.size() * m_each_hex_field);

  return true;
}

bool wex::hexmode::set(bool on, bool use_modification_markers)
{
  if (m_active == on)
    return false;

  m_stc->use_modification_markers(false);

  const bool modified = (m_stc->GetModify());

  on ? activate() : deactivate();

  if (!modified)
  {
    m_stc->EmptyUndoBuffer();
    m_stc->SetSavePoint();
  }

  m_stc->use_modification_markers(use_modification_markers);

  return true;
}

void wex::hexmode::set_pos(const wxKeyEvent& event)
{
  hexmode_line(this).set_pos(event);
}

void wex::hexmode::set_text(const std::string text)
{
  if (!m_active)
    return;

  m_buffer.clear();
  m_buffer_original.clear();

  m_stc->SelectNone();
  m_stc->position_save();
  m_stc->clear(false);

  append_text(text);

  m_stc->position_restore();
}

bool wex::hexmode::sync()
{
  if (!m_active)
  {
    return false;
  }

  set_text(m_buffer);

  return true;
}

void wex::hexmode::undo()
{
  if (m_active)
  {
    m_buffer = m_buffer_original;
  }

  // For hex mode the first min_size bytes should be hex fields (or space).
  const int min_size = m_bytes_per_line * m_each_hex_field;

  m_active =
    (m_stc->GetTextLength() > min_size &&
     std::regex_match(
       m_stc->GetTextRange(0, min_size).ToStdString(),
       std::regex("([0-9A-F][0-9A-F] )+ *")));
}
