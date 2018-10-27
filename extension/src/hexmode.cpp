////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wex::hexmode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/numdlg.h>
#include <wx/spinctrl.h>
#include <wex/hexmode.h>
#include <wex/config.h>
#include <wex/item.h>
#include <wex/itemdlg.h>
#include <wex/lexers.h>
#include <wex/stc.h>
#include "hexmodeline.h"

int GetHexNumberFromUser(
  const wxString& message, const std::string& caption,
  int value, int min, int max,
  wxWindow *parent)
{
  wex::item::UseConfig(false);
  wex::item_dialog dlg({{message, min, max, value}}, wex::window_data().Title(caption).Parent(parent));
  wex::item::UseConfig(true);
  
  ((wxSpinCtrl* )dlg.GetItem(message).GetWindow())->SetBase(16);
  
  return dlg.ShowModal() == wxID_CANCEL ? 
    -1: std::any_cast<int>(dlg.GetItemValue(message));
}

const std::string MakeLine(wex::stc* stc, const std::string& buffer, 
  size_t offset, size_t bytesPerLine, size_t eachHexField)
{
  std::string field_hex, field_ascii;
  
  auto count = buffer.size() - offset;
  count = (bytesPerLine < count ? bytesPerLine : count);

  for (size_t byte = 0; byte < count; byte++)
  {
    const unsigned char c = buffer[offset + byte];

    char buff[4];
    sprintf(buff, "%02X ", c);
    field_hex += buff;
    field_ascii += wex::printable(c, stc);
  }

  const auto field_spaces = std::string(
    (bytesPerLine - count)* eachHexField,
    ' ');
      
  return field_hex + field_spaces + field_ascii +
    (buffer.size() - offset > bytesPerLine ?
       stc->GetEOL(): std::string());
}

// If bytesPerLine is changed, update Convert.
wex::hexmode::hexmode(wex::stc* stc, size_t bytesPerLine)
  : m_STC(stc)
  , m_BytesPerLine(bytesPerLine)
  , m_EachHexField(3)
{
}
  
void wex::hexmode::Activate()
{
  m_Active = true;

  m_STC->SetControlCharSymbol('.');
  m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  m_STC->SetViewEOL(false);
  m_STC->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  m_STC->BeginUndoAction();
  
  wxCharBuffer buffer(m_STC->GetTextRaw());
  SetText(std::string(buffer.data(), buffer.length()));
  
  lexers::Get()->Apply(m_STC);
}

void wex::hexmode::AppendText(const std::string& buffer)
{
  if (!m_Active) return;

  m_Buffer += buffer;
  m_BufferOriginal = m_Buffer;

  std::string text;

  text.reserve(
    // offset:
    (1 + 1) * buffer.size() / m_BytesPerLine + 
    // hex field:
    m_EachHexField * buffer.size() + 
    // ascii field:
    buffer.size());

  for (size_t offset = 0; offset < buffer.size(); offset += m_BytesPerLine)
  {
    text += MakeLine(m_STC, buffer, offset, m_BytesPerLine, m_EachHexField);
  }

  m_STC->AppendTextRaw(text.data(), text.size());
}

void wex::hexmode::ControlCharDialog(const std::string& caption)
{
  if (hexmode_line ml(this, m_STC->GetSelectionStart());
    ml.IsAsciiField() &&
    m_STC->GetSelectedText().size() == 1)
  {
    const wxUniChar value = m_STC->GetSelectedText().GetChar(0);
    
    if (int new_value; (new_value = GetHexNumberFromUser(_("Input") + " 00 - FF",
      caption, value, 0, 255, m_STC)) >= 0)
    {
      ml.Replace(new_value);
    }
  }
  else if (
    ml.IsHexField() &&
    m_STC->GetSelectedText().size() == 2)
  {
    long value;
    
    if (!m_STC->GetSelectedText().ToLong(&value, 16))
    {
      return;
    }

    long new_value;
    if ((new_value = GetHexNumberFromUser(_("Input") + " 00 - FF",
      caption, value, 0, 255, m_STC)) >= 0)
    {
      ml.ReplaceHex(new_value);
    }
  }
}
    
void wex::hexmode::Deactivate() 
{
  m_Active = false;

  m_STC->EndUndoAction();
  m_STC->SetControlCharSymbol(0);
  m_STC->SetEdgeMode(config(_("Edge line")).get(wxSTC_EDGE_NONE));
  m_STC->SetViewEOL(config(_("End of line")).get(false));
  m_STC->SetViewWhiteSpace(config(_("Whitespace visible")).get(wxSTC_WS_INVISIBLE));
  m_STC->ClearDocument(false);
  m_STC->AppendTextRaw(m_Buffer.data(), m_Buffer.size());
  m_STC->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
}

bool wex::hexmode::Delete(int count, int pos) 
{
  return pos == -1 ? 
    hexmode_line(this).Delete(count):
    hexmode_line(this, pos).Delete(count);
}
  
const std::string wex::hexmode::GetInfo()
{
  return hexmode_line(this).GetInfo();
}
  
bool wex::hexmode::GotoDialog()
{
  long val;
  if ((val = wxGetNumberFromUser(
    _("Input") + wxString::Format(" 0 - %d:", m_Buffer.size() - 1),
    wxEmptyString,
    _("Enter Byte Offset"),
    m_Goto, // initial value
    0,
    m_Buffer.size() - 1,
    m_STC)) < 0)
  {
    return false;
  }

  m_Goto = val;
  
  return hexmode_line(this, val, false).Goto();
}

bool wex::hexmode::HighlightOther()
{
  if (const auto pos = m_STC->GetCurrentPos(); HighlightOther(pos))
  {
    return true;
  }
  else if (m_STC->PositionFromLine(pos) != pos)
  {
    return HighlightOther(pos - 1);
  }
  
  return false;
}
    
bool wex::hexmode::HighlightOther(int pos)
{
  if (const auto brace_match = hexmode_line(this, pos).OtherField();
    brace_match != wxSTC_INVALID_POSITION)
  {
    m_STC->BraceHighlight(pos, 
      m_STC->PositionFromLine(m_STC->LineFromPosition(pos)) + brace_match);
    return true;
  }
  else
  {
    m_STC->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
    return false;
  }
}

bool wex::hexmode::Insert(const std::string& text, int pos)
{
  return pos == -1 ? 
    hexmode_line(this).Insert(text):
    hexmode_line(this, pos).Insert(text);
}
  
bool wex::hexmode::Replace(char c, int pos)
{
  return pos == -1 ? 
    hexmode_line(this).Replace(c):
    hexmode_line(this, pos).Replace(c);
}
  
bool wex::hexmode::ReplaceTarget(const std::string& replacement, bool settext)
{
  if (m_STC->GetTargetStart() == wxSTC_INVALID_POSITION || 
      m_STC->GetTargetEnd() == wxSTC_INVALID_POSITION || 
      m_STC->GetTargetEnd() <= m_STC->GetTargetStart() ||
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
  auto start = m_STC->GetTargetStart(); 

  for (
    int i = 0;
    i < (int)replacement.size(); 
    i = i + 2, start = start + m_EachHexField)
  {
    // replace
    if (start <= m_STC->GetTargetEnd())
    {
      hexmode_line(this, start).Replace(
        {replacement[i], replacement[i + 1]}, settext);
    }
    // insert
    else
    {
      hexmode_line(this, start).Insert({replacement[i], replacement[i + 1]});
    }
  }
  
  // delete
  if (start < m_STC->GetTargetEnd())
  {
    hexmode_line(this, start).Delete(
      (m_STC->GetTargetEnd() - start) /  m_EachHexField, settext);
  }

  m_STC->SetTargetEnd(m_STC->GetTargetStart() + replacement.size() * m_EachHexField);
  
  return true;
}
  
bool wex::hexmode::Set(bool on)
{
  if (m_Active == on) return false;

  m_STC->UseModificationMarkers(false);
  
  const bool modified = (m_STC->GetModify());
  
  on ? Activate(): Deactivate();
  
  if (!modified)
  {
    m_STC->EmptyUndoBuffer();
    m_STC->SetSavePoint();
  }
    
  m_STC->UseModificationMarkers(true);
  
  return true;
}

void wex::hexmode::SetPos(const wxKeyEvent& event)
{
  hexmode_line(this).SetPos(event);
}
      
void wex::hexmode::SetText(const std::string text)
{
  if (!m_Active) return;

  m_Buffer.clear();
  m_BufferOriginal.clear();

  m_STC->SelectNone();
  m_STC->PositionSave();
  m_STC->ClearDocument(false);
  
  AppendText(text);

  m_STC->PositionRestore();
}
  
void wex::hexmode::Undo()
{
  if (m_Active)
  {
    m_Buffer = m_BufferOriginal;
  }
  
  // For hex mode the first min_size bytes should be hex fields (or space).
  const int min_size = m_BytesPerLine * m_EachHexField;

  m_Active = (
    m_STC->GetTextLength() > min_size && 
    std::regex_match(
      m_STC->GetTextRange(0, min_size).ToStdString(), 
      std::regex("([0-9A-F][0-9A-F] )+ *")));
}
