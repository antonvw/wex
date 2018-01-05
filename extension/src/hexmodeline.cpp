////////////////////////////////////////////////////////////////////////////////
// Name:      hexmodeline.cpp
// Purpose:   Implementation of class wxExHexModeLine
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stc.h>
#include "hexmodeline.h"

char Printable(unsigned int c, wxExSTC* stc) 
{
  // We do not want control chars (\n etc.) to be printed,
  // as that disturbs the hex view field.
  if (isascii(c) && !iscntrl(c))
  {
    return c;
  }
  else
  {
    // If we already defined our own symbol, use that one,
    // otherwise print an ordinary ascii char.
    const int symbol = stc->GetControlCharSymbol();
    return symbol == 0 ?  '.': symbol;
  }
}

wxExHexModeLine::wxExHexModeLine(wxExHexMode* hex)
  : m_Line(hex->GetSTC()->GetCurLine())
  , m_LineNo(hex->GetSTC()->GetCurrentLine())
  , m_ColumnNo(hex->GetSTC()->GetColumn(hex->GetSTC()->GetCurrentPos()))
  , m_Hex(hex)
  , m_StartAsciiField(hex->m_EachHexField * hex->m_BytesPerLine)
{
  wxASSERT(m_Hex->Active());
}  

wxExHexModeLine::wxExHexModeLine(wxExHexMode* hex, 
  int pos_or_offset, bool is_position)
  : m_Hex(hex)
  , m_StartAsciiField(hex->m_EachHexField * hex->m_BytesPerLine)
{
  wxASSERT(m_Hex->Active());
  
  if (is_position)
  {
    const int pos = (pos_or_offset != -1 ? 
      pos_or_offset: m_Hex->GetSTC()->GetCurrentPos()); 

    m_ColumnNo = m_Hex->GetSTC()->GetColumn(pos);
    m_LineNo = m_Hex->GetSTC()->LineFromPosition(pos);

    if (m_ColumnNo >= m_StartAsciiField + m_Hex->m_BytesPerLine)
    {
      m_ColumnNo = m_StartAsciiField;
      m_LineNo++;
    }

    m_Line = m_Hex->GetSTC()->GetLine(m_LineNo);
  }
  else
  {
    m_Hex->GetSTC()->GotoLine(pos_or_offset >> 4);
    m_Hex->GetSTC()->SelectNone();
    m_ColumnNo = (pos_or_offset & 0x0f);
    m_LineNo = m_Hex->GetSTC()->GetCurrentLine();
    m_Line = m_Hex->GetSTC()->GetLine(m_LineNo);
  }
}

bool wxExHexModeLine::Delete(int count, bool settext)
{
  const int index = GetBufferIndex();
  
  if (IsReadOnly() || 
    index == wxSTC_INVALID_POSITION || 
    (size_t)index >= m_Hex->m_Buffer.length()) return false;

  m_Hex->m_Buffer.erase(index, count);
  
  if (settext)
  {
    m_Hex->SetText(m_Hex->m_Buffer);
  }
  
  return true;
}

int wxExHexModeLine::GetBufferIndex() const
{
  if (m_ColumnNo >= m_StartAsciiField + m_Hex->m_BytesPerLine)
  {
    return wxSTC_INVALID_POSITION;
  }
  else if (m_ColumnNo >= m_StartAsciiField)
  {
    return Convert(m_ColumnNo - m_StartAsciiField);
  }
  else if (m_ColumnNo >= 0)
  {
    if (m_Line[m_ColumnNo] != ' ')
    {
      return Convert(m_ColumnNo / m_Hex->m_EachHexField);
    }
  }

  return wxSTC_INVALID_POSITION;
}
  
const std::string wxExHexModeLine::GetInfo() const
{
  if (IsHexField())
  {
    const std::string word = m_Hex->GetSTC()->GetWordAtPos(m_Hex->GetSTC()->GetCurrentPos());
    
    if (!word.empty())
    {
      return std::string("index: ") + 
        std::to_string(GetBufferIndex()) + std::string(" ") + std::to_string(std::stol(word, 0, 16));
    }
  }
  else if (IsAsciiField())
  {
    return std::string("index: ") + std::to_string(GetBufferIndex());
  }
  
  return std::string();
}

bool wxExHexModeLine::Goto() const
{
  if (m_LineNo < 0 || m_ColumnNo < 0) return false;
  
  m_Hex->GetSTC()->SetFocus(); 
  m_Hex->GetSTC()->SetCurrentPos(
    m_Hex->GetSTC()->PositionFromLine(m_LineNo) + m_StartAsciiField + m_ColumnNo);
  m_Hex->GetSTC()->SelectNone();
  
  return true;
}

bool wxExHexModeLine::Insert(const std::string& text)
{
  const int index = GetBufferIndex();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return false;
  
  if (m_ColumnNo >= m_StartAsciiField)
  {
    m_Hex->m_Buffer.insert(index, text);
    m_Hex->SetText(m_Hex->m_Buffer);

    if (m_ColumnNo + text.size() >= m_Hex->m_BytesPerLine + m_StartAsciiField)
    {
      int line_no = m_Hex->GetSTC()->LineFromPosition(
        m_Hex->GetSTC()->GetCurrentPos()) + 1;
      m_Hex->GetSTC()->SetCurrentPos(
        m_Hex->GetSTC()->PositionFromLine(line_no) + m_StartAsciiField);
    }
    else
    {
      m_Hex->GetSTC()->SetCurrentPos(
        m_Hex->GetSTC()->GetCurrentPos() + text.size());
    }

    m_Hex->GetSTC()->SelectNone();
  }
  else
  {
    if (text.size() != 2 || 
       (!isxdigit(text[0]) && !isxdigit(text[1]))) return false;

    m_Hex->m_Buffer.insert(index, 1, std::stoi(text.substr(0, 2), nullptr, 16));
    m_Hex->SetText(m_Hex->m_Buffer);
  }

  return true;
}
  
bool wxExHexModeLine::Replace(char c)
{
  const int index = GetBufferIndex();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return false;
  
  const int pos = m_Hex->GetSTC()->PositionFromLine(m_LineNo);
  
  // Because m_Buffer is changed, begin and end undo action
  // cannot be used, as these do not operate on the hex buffer.
  
  if (IsAsciiField())
  {
    // replace ascii field with value
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + m_ColumnNo, pos + m_ColumnNo + 1, c);       

    // replace hex field with code
    char buffer[3];
    sprintf(buffer, "%02X", c);
    
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + OtherField(), pos + OtherField() + 2, buffer);
  }
  else if (IsHexField())
  {
    // hex text should be entered.
    if (!isxdigit(c)) return false;
      
    // replace hex field with value
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + m_ColumnNo, pos + m_ColumnNo + 1, c);
        
    // replace ascii field with code
    std::string hex;
    
    if (m_Line[m_ColumnNo + 1] == ' ')
    {
      hex += m_Line[m_ColumnNo - 1];
      hex += c;
    }
    else
    {
      hex += c;
      hex += m_Line[m_ColumnNo];
    }
    
    const int code = std::stoi(hex, nullptr, 16);
    
    m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
      pos + OtherField(), pos + OtherField() + 1, Printable(code, m_Hex->GetSTC()));
      
    c = code;
  }
  else
  {
    return false;
  }

  m_Hex->m_Buffer[index] = c;
  
  return true;
}

void wxExHexModeLine::Replace(const std::string& hex, bool settext)
{
  const int index = GetBufferIndex();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return;
  
  m_Hex->m_Buffer[index] = std::stoi(hex, nullptr, 16);

  if (settext)
  {
    m_Hex->SetText(m_Hex->m_Buffer);
  }
}
  
void wxExHexModeLine::ReplaceHex(int value)
{
  const int index = GetBufferIndex();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return;
  
  const int pos = m_Hex->GetSTC()->PositionFromLine(m_LineNo);
  
  char buffer[2];
  sprintf(buffer, "%X", value);
  
  // replace hex field with value
  m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
    pos + m_ColumnNo, pos + m_ColumnNo + 2, buffer);
        
  // replace ascii field with code
  m_Hex->GetSTC()->wxStyledTextCtrl::Replace(
    pos + OtherField(), pos + OtherField() + 1, Printable(value, m_Hex->GetSTC()));
      
  m_Hex->m_Buffer[index] = value;
}

void wxExHexModeLine::SetPos(const wxKeyEvent& event)
{
  const int start = m_Hex->GetSTC()->PositionFromLine(m_LineNo);
  const bool right = (event.GetKeyCode() == WXK_RIGHT);
  const int pos = m_Hex->GetSTC()->GetCurrentPos();
  
  if (IsHexField())
  {
    right ? 
      m_Hex->GetSTC()->SetCurrentPos(pos + 3):
      m_Hex->GetSTC()->SetCurrentPos(pos - 3);
      
    if (m_Hex->GetSTC()->GetCurrentPos() >= start + m_StartAsciiField)
    {
      m_Hex->GetSTC()->SetCurrentPos(
        m_Hex->GetSTC()->PositionFromLine(m_LineNo + 1));
    }
    else if (m_Hex->GetSTC()->GetCurrentPos() < start)
    {
      if (m_LineNo > 0)
      {
        m_Hex->GetSTC()->SetCurrentPos(
          m_Hex->GetSTC()->PositionFromLine(m_LineNo - 1) 
            + m_StartAsciiField - m_Hex->m_EachHexField);
      }
    }
  }
  else
  {
    right ? 
      m_Hex->GetSTC()->SetCurrentPos(pos + 1):
      m_Hex->GetSTC()->SetCurrentPos(pos - 1);
      
    if (m_Hex->GetSTC()->GetCurrentPos() >= start + m_StartAsciiField + m_Hex->m_BytesPerLine)
    {
      m_Hex->GetSTC()->SetCurrentPos(
        m_Hex->GetSTC()->PositionFromLine(m_LineNo + 1) + m_StartAsciiField);
    }
    else if (m_Hex->GetSTC()->GetCurrentPos() < start + m_StartAsciiField)
    {
      if (m_LineNo > 0)
      {
        m_Hex->GetSTC()->SetCurrentPos(m_Hex->GetSTC()->GetLineEndPosition(m_LineNo - 1) - 1);
      }
    }
  }
}
