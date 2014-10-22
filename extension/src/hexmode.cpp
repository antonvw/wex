////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wxExHexMode and wxExHexModeLine
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

const wxFileOffset bytes_per_line = 16;
const wxFileOffset each_hex_field = 3;
const wxFileOffset start_hex_field = 9;
const wxFileOffset start_ascii_field =
  start_hex_field + each_hex_field * bytes_per_line;
  
wxExHexMode::wxExHexMode(wxExSTC* stc)
  : m_STC(stc)
  , m_Active(false)
  , m_Goto(0)
{
}
  
void wxExHexMode::Activate(const wxCharBuffer& text) 
{
  Clear();
  
  m_STC->SetControlCharSymbol('.');

  // Do not show an edge, eol or whitespace in hex mode.
  m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  m_STC->SetViewEOL(false);
  m_STC->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  m_STC->ClearDocument(false);
  
  AppendText(text);
    
  wxExLexers::Get()->ApplyHexStyles(m_STC);
  wxExLexers::Get()->ApplyMarkers(m_STC);
  
  m_Active = true;
}

void wxExHexMode::AppendText(const wxCharBuffer& buffer)
{
  wxFileOffset start = m_Buffer.length();
  
  m_Buffer += buffer;
  m_BufferOriginal = m_Buffer;
  
  wxString text;

  // Allocate space for the string.
  text.Alloc(
    // offset:      (9 + 1 + 1) * length / 16 bytes, 
    (start_hex_field + 1 + 1) * buffer.length() / bytes_per_line + 
    // hex field:   3 * length 
    each_hex_field * buffer.length() + 
    // ascii field: just the length
    buffer.length());

  // Using wxString::Format here asserts (wxWidgets-2.9.1).
  char field_offset[start_hex_field + 1];
  
  for (
    wxFileOffset offset = 0; 
    offset < buffer.length(); 
    offset += bytes_per_line)
  {
    long count = buffer.length() - offset;
    count =
      (bytes_per_line < count ? bytes_per_line : count);

    wxString field_hex, field_ascii;

    for (register wxFileOffset byte = 0; byte < count; byte++)
    {
      const unsigned char c = buffer.data()[offset + byte];

      field_hex += wxString::Format("%02X ", c);
      field_ascii += Printable(c);
    }

    sprintf(field_offset, "%08lX ", (unsigned long)start + (unsigned long)offset);
    
    const wxString field_spaces = wxString(
      ' ', 
      (bytes_per_line - count)* each_hex_field);

    text +=  
      field_offset + 
      field_hex +
      field_spaces +
      field_ascii;
      
    if (buffer.length() - offset > bytes_per_line)
    {
      text += m_STC->GetEOL();
    }
  }

  m_STC->AppendText(text);
}

void wxExHexMode::Clear()
{
  m_Buffer.clear();
  m_BufferOriginal.clear();
}

void wxExHexMode::ControlCharDialog(const wxString& caption)
{
  wxExHexModeLine ml(this, m_STC->GetSelectionStart());
  
  if (
    ml.IsAsciiField() &&
    m_STC->GetSelectedText().length() == 1)
  {
    const wxUniChar value = m_STC->GetSelectedText().GetChar(0);

    long new_value;
    if ((new_value = wxExGetHexNumberFromUser(_("Input") + " 00 - FF",
      wxEmptyString,
      caption,
      value,
      0,
      255,
      m_STC)) < 0)
    {
      return;
    }
    
    ml.Replace(new_value);
  }
  else if (
    ml.IsHexField() &&
    m_STC->GetSelectedText().length() == 2)
  {
    long value;
    
    if (!m_STC->GetSelectedText().ToLong(&value, 16))
    {
      return;
    }

    long new_value;
    if ((new_value = wxExGetHexNumberFromUser(_("Input") + " 00 - FF",
      wxEmptyString,
      caption,
      value,
      0,
      255,
      m_STC)) < 0)
    {
      return;
    }
    
    ml.ReplaceHex(new_value);
  }
}
    
void wxExHexMode::Deactivate() 
{
  m_STC->SetControlCharSymbol(0);
  
  m_STC->SetEdgeMode(wxConfigBase::Get()->ReadLong(_("Edge line"), wxSTC_EDGE_NONE));
  m_STC->SetViewEOL(wxConfigBase::Get()->ReadBool(_("End of line"), false));
  m_STC->SetViewWhiteSpace(wxConfigBase::Get()->ReadLong(_("Whitespace"), wxSTC_WS_INVISIBLE));
  
  m_STC->ClearDocument(false);
  m_STC->AppendText(m_Buffer);
  m_STC->BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
  
  Clear();
  
  m_Active = false;
}

bool wxExHexMode::GotoDialog()
{
  long val;
  if ((val = wxGetNumberFromUser(
    _("Input") + wxString::Format(" 0 - %d:", m_Buffer.length() - 1),
    wxEmptyString,
    _("Enter Byte Offset"),
    m_Goto, // initial value
    0,
    m_Buffer.length() - 1,
    m_STC)) < 0)
  {
    return false;
  }

  m_Goto = val;
  
  wxExHexModeLine(this, val, false).Goto();
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

wxUniChar wxExHexMode::Printable(unsigned int c) const
{
  // We do not want control chars (\n etc.) to be printed,
  // as that disturbs the hex view field.
  if (wxIsascii(c) && !iscntrl(c))
  {
    return c;
  }
  else
  {
    // If we already defined our own symbol, use that one,
    // otherwise print an ordinary ascii char.
    const int symbol = m_STC->GetControlCharSymbol();
    
    if (symbol == 0)
    {
      return '.';
    }
    else
    {
      return symbol;
    }
  }
}
  
bool wxExHexMode::Set(
  bool on, 
  const wxCharBuffer& text)
{
  if (m_STC->GetVi().GetMode() == wxExVi::MODE_INSERT)
  {
    return false;
  }
  
  if (
    ( m_Active && on) ||
    (!m_Active && !on))
  {
    return true;
  }
    
  m_STC->UseModificationMarkers(false);
  
  const bool modified = (m_STC->GetModify());
  
  m_STC->BeginUndoAction();
  
  if (on) 
  {
    Activate(text); 
  }
  else
  {
    Deactivate();
  }
  
  m_STC->EndUndoAction();
  
  if (!modified)
  {
    m_STC->EmptyUndoBuffer();
    m_STC->SetSavePoint();
  }
    
  m_STC->UseModificationMarkers(true);

  return true;
}

void wxExHexMode::SetBuffer(int byte, int value)
{
  m_Buffer[byte] = value;
}

void wxExHexMode::Undo()
{
  if (m_Active)
  {
    m_Buffer = m_BufferOriginal;
  }
  
  // Check the mode we are in.
  m_Active = (
    m_STC->GetTextLength() > 60 && m_STC->GetTextRange(0, 9) == "00000000 ");
}
      
wxExHexModeLine::wxExHexModeLine(wxExHexMode* hex)
  : m_Line(hex->GetSTC()->GetCurLine())
  , m_LineNo(hex->GetSTC()->GetCurrentLine())
  , m_Index(hex->GetSTC()->GetColumn(hex->GetSTC()->GetCurrentPos()))
  , m_Hex(hex)
{
  wxASSERT(m_Hex->Active());
}  

wxExHexModeLine::wxExHexModeLine(wxExHexMode* hex, 
  int pos_or_offset, bool is_position)
  : m_Hex(hex)
{
  wxASSERT(m_Hex->Active());
  
  if (is_position)
  {
    Set(pos_or_offset != -1 ? pos_or_offset: m_Hex->GetSTC()->GetCurrentPos()); 
  }
  else
  {
    char field_offset[start_hex_field];
  
    sprintf(field_offset, "%08lX", (unsigned long)(pos_or_offset & ~0x0f));
  
    if (m_Hex->GetSTC()->FindNext(field_offset, 0))
    {
      m_LineNo = m_Hex->GetSTC()->GetCurrentLine();
      m_Line = m_Hex->GetSTC()->GetLine(m_LineNo);
      m_Index = (pos_or_offset & 0x0f);
      m_Hex->GetSTC()->SelectNone();
    }
    else
    {
      m_LineNo = -1;
      m_Line.clear();
      m_Index = -1;
    }
  }
}

int wxExHexModeLine::Convert(int offset) const
{
  const wxString address(m_Line.Mid(0, start_hex_field));
  
  long val;
  
  if (sscanf(address.c_str(), "%lx", &val))
  {
    return val + offset;
  }
  
  wxLogError("Cannot convert hex: " + address + " to number");
      
  return 0;
}

int wxExHexModeLine::GetAsciiField() const
{
  if (m_Line.GetChar(m_Index) != ' ')
  {
    const int offset = (m_Index - (start_hex_field)) / each_hex_field;

    return start_ascii_field + offset;
  }
  
  return wxSTC_INVALID_POSITION;
}

int wxExHexModeLine::GetByte() const
{
  if (m_Index > start_ascii_field + bytes_per_line)
  {
    return wxSTC_INVALID_POSITION;
  }
  else if (m_Index >= start_ascii_field)
  {
    return Convert(m_Index - start_ascii_field);
  }
  else if (m_Index >= start_hex_field)
  {
    if (m_Line.GetChar(m_Index) != ' ')
    {
      return Convert((m_Index - (start_hex_field)) / each_hex_field);
    }
  }

  return wxSTC_INVALID_POSITION;
}
  
const wxString wxExHexModeLine::GetInfo() const
{
  if (IsHexField() || IsOffsetField())
  {
    const wxString word = m_Hex->GetSTC()->GetWordAtPos(m_Hex->GetSTC()->GetCurrentPos());
    
    if (!word.empty())
    {
      long base16_val;
      const bool base16_ok = word.ToLong(&base16_val, 16);

      if (base16_ok)
      {
        if (IsOffsetField())
        {
          return wxString::Format("%ld", base16_val);
        }
        else
        {
          return wxString::Format("byte: %d %ld", GetByte(), base16_val);
        }
      }
    }
  }
  else if (IsAsciiField())
  {
    return wxString::Format("byte: %d", GetByte());
  }
  
  return wxEmptyString;
}

int wxExHexModeLine::GetHexField() const
{
  const int offset = m_Index - start_ascii_field;

  return start_hex_field + each_hex_field * offset;
}

bool wxExHexModeLine::Goto() const
{
  if (m_LineNo < 0 || m_Index < 0)
  {
    return false;
  }
  
  const int start = m_Hex->GetSTC()->PositionFromLine(m_LineNo);
 
  m_Hex->GetSTC()->SetFocus(); 
  m_Hex->GetSTC()->SetCurrentPos(start + start_ascii_field + m_Index);
  m_Hex->GetSTC()->SelectNone();
  
  return true;
}

bool wxExHexModeLine::IsAsciiField() const
{
  return 
    m_Index >= start_ascii_field && 
    m_Index < start_ascii_field + bytes_per_line;
}

bool wxExHexModeLine::IsHexField() const
{
  return 
    m_Index >= start_hex_field &&
    m_Index < start_ascii_field;
}

bool wxExHexModeLine::IsOffsetField() const
{
  return 
    m_Index >= 0 &&
    m_Index < start_hex_field;
}

bool wxExHexModeLine::IsReadOnly() const
{
  if (IsAsciiField())
  {
    return false;
  }
  else if (IsHexField())
  {
    if (m_Line.GetChar(m_Index) != ' ')
    {
      return false;
    }
  }
  
  return true;
}

int wxExHexModeLine::OtherField() const
{
  if (IsAsciiField())
  {
    return GetHexField();
  }
  else if (IsHexField())
  {
    return GetAsciiField();
  }
  else
  {
    return wxSTC_INVALID_POSITION;
  }
}

bool wxExHexModeLine::Replace(const wxUniChar& c)
{
  if (IsReadOnly() || m_LineNo < 0 || m_Index < 0)
  {
    return false;
  }
  
  const int byte = GetByte();
  
  if (byte == wxSTC_INVALID_POSITION)
  {
    return false;
  }
  
  const int pos = m_Hex->GetSTC()->PositionFromLine(m_LineNo);
  
  wxUniChar val = c;
  
  // Because m_Buffer is changed, begin and end undo action
  // cannot be used, as these do not operate on the hex buffer.
  
  if (IsAsciiField())
  {
    // replace ascii field with value
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + m_Index, 
      pos + m_Index + 1, 
      c);
      
    // replace hex field with code
    const wxString code = wxString::Format("%02X", c);
    
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + OtherField(), 
      pos + OtherField() + 2, 
      code);
  }
  else if (IsHexField())
  {
    // hex text should be entered.
    if (!isxdigit(c))
    {
      return false;
    }
      
    // replace hex field with value
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + m_Index, 
      pos + m_Index + 1,
      c);
        
    // replace ascii field with code
    char str[3];
    
    if (m_Line[m_Index + 1] == ' ')
    {
      str[0] = m_Line[m_Index - 1];
      str[1] = c;
    }
    else
    {
      str[0] = c;
      str[1] = m_Line[m_Index];
    }
    
    str[2] = '\0';
    
    unsigned int code;
    sscanf(str, "%X", &code);
    
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + OtherField(), 
      pos + OtherField() + 1, 
      m_Hex->Printable(code));
      
    val = code;
  }
  else
  {
    return false;
  }

  m_Hex->SetBuffer(byte, val);
  
  return true;
}

bool wxExHexModeLine::ReplaceHex(int value)
{
  if (IsReadOnly() || !IsHexField() || m_LineNo < 0 || m_Index < 0)
  {
    return false;
  }
  
  const int byte = GetByte();
  
  if (byte == wxSTC_INVALID_POSITION)
  {
    return false;
  }
  
  const int pos = m_Hex->GetSTC()->PositionFromLine(m_LineNo);
  
  wxString buffer = wxString::Format("%X", value);
  
  // replace hex field with value
  m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
    pos + m_Index, 
    pos + m_Index + 2,
    buffer);
        
  // replace ascii field with code
  m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
    pos + OtherField(), 
    pos + OtherField() + 1, 
    m_Hex->Printable(value));
      
  m_Hex->SetBuffer(byte, value);
  
  return true;
}

void wxExHexModeLine::Set(int pos)
{
  wxASSERT(m_Hex->Active());
  
  m_LineNo = m_Hex->GetSTC()->LineFromPosition(pos);
  m_Line = m_Hex->GetSTC()->GetLine(m_LineNo);
  m_Index = m_Hex->GetSTC()->GetColumn(pos);
}

void wxExHexModeLine::SetPos(const wxKeyEvent& event)
{
  wxASSERT(m_Hex->Active());
  
  const int start = m_Hex->GetSTC()->PositionFromLine(m_LineNo);
  const bool right = (event.GetKeyCode() == WXK_RIGHT);
  const int pos = m_Hex->GetSTC()->GetCurrentPos();
  
  if (IsOffsetField())
  {
    if (right)
    {
      m_Hex->GetSTC()->SetCurrentPos(start + start_hex_field);
    }
    else
    {
      if (m_LineNo > 0)
      {
        m_Hex->GetSTC()->SetCurrentPos(
          m_Hex->GetSTC()->PositionFromLine(m_LineNo - 1) + start_hex_field);
      }
    }
  }
  else if (IsHexField())
  {
    right ? 
      m_Hex->GetSTC()->SetCurrentPos(pos + 3):
      m_Hex->GetSTC()->SetCurrentPos(pos - 3);
      
    if (m_Hex->GetSTC()->GetCurrentPos() >= start + start_ascii_field)
    {
      m_Hex->GetSTC()->SetCurrentPos(
        m_Hex->GetSTC()->PositionFromLine(m_LineNo + 1) + start_hex_field);
    }
    else if (m_Hex->GetSTC()->GetCurrentPos() < start + start_hex_field)
    {
      if (m_LineNo > 0)
      {
        m_Hex->GetSTC()->SetCurrentPos(
          m_Hex->GetSTC()->PositionFromLine(m_LineNo - 1) 
            + start_ascii_field - each_hex_field);
      }
    }
  }
  else
  {
    right ? 
      m_Hex->GetSTC()->SetCurrentPos(pos + 1):
      m_Hex->GetSTC()->SetCurrentPos(pos - 1);
      
    if (m_Hex->GetSTC()->GetCurrentPos() >= start + start_ascii_field + bytes_per_line)
    {
      m_Hex->GetSTC()->SetCurrentPos(
        m_Hex->GetSTC()->PositionFromLine(m_LineNo + 1) + start_ascii_field);
    }
    else if (m_Hex->GetSTC()->GetCurrentPos() < start + start_ascii_field)
    {
      if (m_LineNo > 0)
      {
        m_Hex->GetSTC()->SetCurrentPos(m_Hex->GetSTC()->GetLineEndPosition(m_LineNo - 1) - 1);
      }
    }
  }
}
