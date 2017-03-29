////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wxExHexMode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/spinctrl.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/item.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include "hexmodeline.h"

int GetHexNumberFromUser(
  const wxString& message, const std::string& caption,
  int value, int min, int max,
  wxWindow *parent)
{
  wxExItem::UseConfig(false);

  wxExItemDialog dlg(parent, {{message, min, max, value}}, caption);
  
  wxExItem::UseConfig(true);
  
  ((wxSpinCtrl* )dlg.GetItem(message).GetWindow())->SetBase(16);
  
  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return -1;
  }
  
  return dlg.GetItemValue(message).As<int>();
}

const std::string MakeLine(wxExSTC* stc, const std::string& buffer, 
  wxFileOffset offset, wxFileOffset bytesPerLine, wxFileOffset eachHexField)
{
  std::string field_hex, field_ascii;
  
  long count = buffer.size() - offset;
  count = (bytesPerLine < count ? bytesPerLine : count);

  for (wxFileOffset byte = 0; byte < count; byte++)
  {
    const unsigned char c = buffer[offset + byte];

    char buff[3];
    sprintf(buff, "%02X ", c);
    field_hex += buff;
    field_ascii += Printable(c, stc);
  }

  const std::string field_spaces = std::string(
    (bytesPerLine - count)* eachHexField,
    ' ');
      
  return field_hex + field_spaces + field_ascii +
    (buffer.size() - offset > bytesPerLine ?
       stc->GetEOL(): std::string());
}

// If bytesPerLine is changed, update Convert.
wxExHexMode::wxExHexMode(wxExSTC* stc, wxFileOffset bytesPerLine)
  : m_STC(stc)
  , m_BytesPerLine(bytesPerLine)
  , m_EachHexField(3)
{
}
  
void wxExHexMode::Activate()
{
  m_Active = true;

  m_STC->SetControlCharSymbol('.');
  m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  m_STC->SetViewEOL(false);
  m_STC->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  
  wxCharBuffer buffer(m_STC->GetTextRaw());
  SetText(std::string(buffer.data(), buffer.length()));
  
  wxExLexers::Get()->Apply(m_STC);
}

void wxExHexMode::AppendText(const std::string& buffer)
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

  for (wxFileOffset offset = 0; offset < buffer.size(); offset += m_BytesPerLine)
  {
    text += MakeLine(m_STC, buffer, offset, m_BytesPerLine, m_EachHexField);
  }

  m_STC->AppendTextRaw(text.data(), text.size());
}

void wxExHexMode::ControlCharDialog(const std::string& caption)
{
  wxExHexModeLine ml(this, m_STC->GetSelectionStart());
  
  if (
    ml.IsAsciiField() &&
    m_STC->GetSelectedText().size() == 1)
  {
    const wxUniChar value = m_STC->GetSelectedText().GetChar(0);

    int new_value;
    if ((new_value = GetHexNumberFromUser(_("Input") + " 00 - FF",
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
    
void wxExHexMode::Deactivate() 
{
  m_Active = false;

  m_STC->SetControlCharSymbol(0);
  m_STC->SetEdgeMode(wxConfigBase::Get()->ReadLong(_("Edge line"), wxSTC_EDGE_NONE));
  m_STC->SetViewEOL(wxConfigBase::Get()->ReadBool(_("End of line"), false));
  m_STC->SetViewWhiteSpace(wxConfigBase::Get()->ReadLong(_("Whitespace"), wxSTC_WS_INVISIBLE));
  m_STC->ClearDocument(false);
  m_STC->AppendTextRaw(m_Buffer.data(), m_Buffer.size());
  m_STC->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
}

bool wxExHexMode::Delete(int count, int pos) 
{
  return pos == -1 ? 
    wxExHexModeLine(this).Delete(count):
    wxExHexModeLine(this, pos).Delete(count);
}
  
const std::string wxExHexMode::GetInfo()
{
  return wxExHexModeLine(this).GetInfo();
}
  
bool wxExHexMode::GotoDialog()
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
  
  return wxExHexModeLine(this, val, false).Goto();
}

bool wxExHexMode::HighlightOther()
{
  const int pos = m_STC->GetCurrentPos();
  
  if (HighlightOther(pos))
  {
    return true;
  }
    
  if (m_STC->PositionFromLine(pos) != pos)
  {
    return HighlightOther(pos - 1);
  }
  
  return false;
}
    
bool wxExHexMode::HighlightOther(int pos)
{
  const int brace_match = wxExHexModeLine(this, pos).OtherField();
  
  if (brace_match != wxSTC_INVALID_POSITION)
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

bool wxExHexMode::Insert(const std::string& text, int pos)
{
  return pos == -1 ? 
    wxExHexModeLine(this).Insert(text):
    wxExHexModeLine(this, pos).Insert(text);
}
  
bool wxExHexMode::Replace(char c, int pos)
{
  return pos == -1 ? 
    wxExHexModeLine(this).Replace(c):
    wxExHexModeLine(this, pos).Replace(c);
}
  
bool wxExHexMode::ReplaceTarget(const std::string& replacement, bool settext)
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
  int start = m_STC->GetTargetStart(); 

  for (
    int i = 0;
    i < (int)replacement.size(); 
    i = i + 2, start = start + m_EachHexField)
  {
    // replace
    if (start <= m_STC->GetTargetEnd())
    {
      wxExHexModeLine(this, start).Replace(
        {replacement[i], replacement[i + 1]}, settext);
    }
    // insert
    else
    {
      wxExHexModeLine(this, start).Insert({replacement[i], replacement[i + 1]});
    }
  }
  
  // delete
  if (start < m_STC->GetTargetEnd())
  {
    wxExHexModeLine(this, start).Delete(
      (m_STC->GetTargetEnd() - start) /  m_EachHexField, settext);
  }

  m_STC->SetTargetEnd(m_STC->GetTargetStart() + replacement.size() * m_EachHexField);
  
  return true;
}
  
void wxExHexMode::Set(bool on)
{
  if (m_Active == on) return;

  m_STC->UseModificationMarkers(false);
  
  const bool modified = (m_STC->GetModify());
  
  m_STC->BeginUndoAction();
  
  on ? Activate(): Deactivate();
  
  m_STC->EndUndoAction();
  
  if (!modified)
  {
    m_STC->EmptyUndoBuffer();
    m_STC->SetSavePoint();
  }
    
  m_STC->UseModificationMarkers(true);
}

void wxExHexMode::SetPos(const wxKeyEvent& event)
{
  wxExHexModeLine(this).SetPos(event);
}
      
void wxExHexMode::SetText(const std::string text)
{
  if (!m_Active) return;

  m_Buffer.clear();
  m_BufferOriginal.clear();

  m_STC->BeginUndoAction();
  m_STC->SelectNone();
  m_STC->PositionSave();
  m_STC->ClearDocument(false);
  
  AppendText(text);

  m_STC->PositionRestore();
  m_STC->EndUndoAction();
}
  
void wxExHexMode::Undo()
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
