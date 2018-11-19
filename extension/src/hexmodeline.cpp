////////////////////////////////////////////////////////////////////////////////
// Name:      hexmodeline.cpp
// Purpose:   Implementation of class hexmode_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/stc.h>
#include "hexmodeline.h"

wex::hexmode_line::hexmode_line(hexmode* hex)
  : m_Line(hex->stc()->GetCurLine())
  , m_LineNo(hex->stc()->GetCurrentLine())
  , m_ColumnNo(hex->stc()->GetColumn(hex->stc()->GetCurrentPos()))
  , m_Hex(hex)
  , m_StartAsciiField(hex->m_EachHexField * hex->m_BytesPerLine)
{
  assert(m_Hex->is_active());
}  

wex::hexmode_line::hexmode_line(hexmode* hex, 
  int pos_or_offset, bool is_position)
  : m_Hex(hex)
  , m_StartAsciiField(hex->m_EachHexField * hex->m_BytesPerLine)
{
  assert(m_Hex->is_active());
  
  if (is_position)
  {
    const int pos = (pos_or_offset != -1 ? 
      pos_or_offset: m_Hex->stc()->GetCurrentPos()); 

    m_ColumnNo = m_Hex->stc()->GetColumn(pos);
    m_LineNo = m_Hex->stc()->LineFromPosition(pos);

    if (m_ColumnNo >= m_StartAsciiField + m_Hex->m_BytesPerLine)
    {
      m_ColumnNo = m_StartAsciiField;
      m_LineNo++;
    }

    m_Line = m_Hex->stc()->GetLine(m_LineNo);
  }
  else
  {
    m_Hex->stc()->GotoLine(pos_or_offset >> 4);
    m_Hex->stc()->SelectNone();
    m_ColumnNo = (pos_or_offset & 0x0f);
    m_LineNo = m_Hex->stc()->GetCurrentLine();
    m_Line = m_Hex->stc()->GetLine(m_LineNo);
  }
}

bool wex::hexmode_line::Delete(int count, bool settext)
{
  const int index = buffer_index();
  
  if (IsReadOnly() || 
    index == wxSTC_INVALID_POSITION || 
    (size_t)index >= m_Hex->m_Buffer.length()) return false;

  m_Hex->m_Buffer.erase(index, count);
  
  if (settext)
  {
    m_Hex->set_text(m_Hex->m_Buffer);
  }
  
  return true;
}

int wex::hexmode_line::buffer_index() const
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
  
const std::string wex::hexmode_line::info() const
{
  if (IsHexField())
  {
    const std::string word = m_Hex->stc()->get_word_at_pos(m_Hex->stc()->GetCurrentPos());
    
    if (!word.empty())
    {
      return std::string("index: ") + 
        std::to_string(buffer_index()) + std::string(" ") + std::to_string(std::stol(word, 0, 16));
    }
  }
  else if (IsAsciiField())
  {
    return std::string("index: ") + std::to_string(buffer_index());
  }
  
  return std::string();
}

bool wex::hexmode_line::Goto() const
{
  if (m_LineNo < 0 || m_ColumnNo < 0) return false;
  
  m_Hex->stc()->SetFocus(); 
  m_Hex->stc()->SetCurrentPos(
    m_Hex->stc()->PositionFromLine(m_LineNo) + m_StartAsciiField + m_ColumnNo);
  m_Hex->stc()->SelectNone();
  
  return true;
}

bool wex::hexmode_line::Insert(const std::string& text)
{
  const int index = buffer_index();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return false;
  
  if (m_ColumnNo >= m_StartAsciiField)
  {
    m_Hex->m_Buffer.insert(index, text);
    m_Hex->set_text(m_Hex->m_Buffer);

    if (m_ColumnNo + text.size() >= m_Hex->m_BytesPerLine + m_StartAsciiField)
    {
      int line_no = m_Hex->stc()->LineFromPosition(
        m_Hex->stc()->GetCurrentPos()) + 1;
      m_Hex->stc()->SetCurrentPos(
        m_Hex->stc()->PositionFromLine(line_no) + m_StartAsciiField);
    }
    else
    {
      m_Hex->stc()->SetCurrentPos(
        m_Hex->stc()->GetCurrentPos() + text.size());
    }

    m_Hex->stc()->SelectNone();
  }
  else
  {
    if (text.size() != 2 || 
       (!isxdigit(text[0]) && !isxdigit(text[1]))) return false;

    m_Hex->m_Buffer.insert(index, 1, std::stoi(text.substr(0, 2), nullptr, 16));
    m_Hex->set_text(m_Hex->m_Buffer);
  }

  return true;
}
  
bool wex::hexmode_line::Replace(char c)
{
  const int index = buffer_index();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return false;
  
  const int pos = m_Hex->stc()->PositionFromLine(m_LineNo);
  
  // Because m_Buffer is changed, begin and end undo action
  // cannot be used, as these do not operate on the hex buffer.
  
  if (IsAsciiField())
  {
    // replace ascii field with value
    m_Hex->stc()->wxStyledTextCtrl::Replace(
      pos + m_ColumnNo, pos + m_ColumnNo + 1, c);       

    // replace hex field with code
    char buffer[3];
    sprintf(buffer, "%02X", c);
    
    m_Hex->stc()->wxStyledTextCtrl::Replace(
      pos + OtherField(), pos + OtherField() + 2, buffer);
  }
  else if (IsHexField())
  {
    // hex text should be entered.
    if (!isxdigit(c)) return false;
      
    // replace hex field with value
    m_Hex->stc()->wxStyledTextCtrl::Replace(
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
    
    m_Hex->stc()->wxStyledTextCtrl::Replace(
      pos + OtherField(), pos + OtherField() + 1, wex::printable(code, m_Hex->stc()));
      
    c = code;
  }
  else
  {
    return false;
  }

  m_Hex->m_Buffer[index] = c;
  
  return true;
}

void wex::hexmode_line::Replace(const std::string& hex, bool settext)
{
  const int index = buffer_index();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return;
  
  m_Hex->m_Buffer[index] = std::stoi(hex, nullptr, 16);

  if (settext)
  {
    m_Hex->set_text(m_Hex->m_Buffer);
  }
}
  
void wex::hexmode_line::ReplaceHex(int value)
{
  const int index = buffer_index();
  
  if (IsReadOnly() || index == wxSTC_INVALID_POSITION) return;
  
  const int pos = m_Hex->stc()->PositionFromLine(m_LineNo);
  
  char buffer[2];
  sprintf(buffer, "%X", value);
  
  // replace hex field with value
  m_Hex->stc()->wxStyledTextCtrl::Replace(
    pos + m_ColumnNo, pos + m_ColumnNo + 2, buffer);
        
  // replace ascii field with code
  m_Hex->stc()->wxStyledTextCtrl::Replace(
    pos + OtherField(), pos + OtherField() + 1, printable(value, m_Hex->stc()));
      
  m_Hex->m_Buffer[index] = value;
}

void wex::hexmode_line::SetPos(const wxKeyEvent& event)
{
  const int start = m_Hex->stc()->PositionFromLine(m_LineNo);
  const bool right = (event.GetKeyCode() == WXK_RIGHT);
  const int pos = m_Hex->stc()->GetCurrentPos();
  
  if (IsHexField())
  {
    right ? 
      m_Hex->stc()->SetCurrentPos(pos + 3):
      m_Hex->stc()->SetCurrentPos(pos - 3);
      
    if (m_Hex->stc()->GetCurrentPos() >= start + m_StartAsciiField)
    {
      m_Hex->stc()->SetCurrentPos(
        m_Hex->stc()->PositionFromLine(m_LineNo + 1));
    }
    else if (m_Hex->stc()->GetCurrentPos() < start)
    {
      if (m_LineNo > 0)
      {
        m_Hex->stc()->SetCurrentPos(
          m_Hex->stc()->PositionFromLine(m_LineNo - 1) 
            + m_StartAsciiField - m_Hex->m_EachHexField);
      }
    }
  }
  else
  {
    right ? 
      m_Hex->stc()->SetCurrentPos(pos + 1):
      m_Hex->stc()->SetCurrentPos(pos - 1);
      
    if (m_Hex->stc()->GetCurrentPos() >= start + m_StartAsciiField + m_Hex->m_BytesPerLine)
    {
      m_Hex->stc()->SetCurrentPos(
        m_Hex->stc()->PositionFromLine(m_LineNo + 1) + m_StartAsciiField);
    }
    else if (m_Hex->stc()->GetCurrentPos() < start + m_StartAsciiField)
    {
      if (m_LineNo > 0)
      {
        m_Hex->stc()->SetCurrentPos(m_Hex->stc()->GetLineEndPosition(m_LineNo - 1) - 1);
      }
    }
  }
}

char wex::printable(unsigned int c, wex::stc* stc) 
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
