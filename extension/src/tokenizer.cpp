////////////////////////////////////////////////////////////////////////////////
// Name:      tokenizer.cpp
// Purpose:   Implementation of class wxExTokenizer
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/tokenizer.h>

wxExTokenizer::wxExTokenizer(
  const std::string& text, const std::string& delimiters, bool skip_empty_tokens)
  : m_Delimiters(delimiters)
  , m_SkipEmptyTokens(skip_empty_tokens)
  , m_Text(text)
{
}

size_t wxExTokenizer::CountTokens() const 
{
  size_t count = 0;
  wxExTokenizer tkz(m_Text, m_Delimiters, m_SkipEmptyTokens);

  while (tkz.HasMoreTokens())
  {
    tkz.GetNextToken();
    count++;
  }

  return count;
}

const std::string wxExTokenizer::GetNextToken() 
{
  if (!HasMoreTokens()) return std::string();

  if (m_SkipEmptyTokens)
  {
    m_TokenStartPos = m_Text.find_first_not_of(m_Delimiters, m_TokenEndPos);
    m_StartPos = m_TokenStartPos;
  }
  else
  {
    m_TokenStartPos = m_StartPos;
  }
  
  if (m_TokenStartPos == std::string::npos)
  {
    return std::string();
  }

  m_TokenEndPos = m_Text.find_first_of(m_Delimiters, m_StartPos);
  m_LastDelimiter = (m_TokenEndPos != std::string::npos ? m_Text[m_TokenEndPos]: 0);

  if (!m_SkipEmptyTokens)
  {
    m_StartPos = m_TokenEndPos + 1;
  }

  return GetToken();
}

const std::string wxExTokenizer::GetString() const
{
  if (m_TokenEndPos == std::string::npos)
  {
    return std::string();
  }

  size_t pos = m_TokenEndPos;

  // skip leading delimiters
  while (pos < m_Text.size() && m_Delimiters.find(m_Text[pos]) != std::string::npos)
  {
    pos++;
  }
  
  return m_Text.substr(pos);
}
  
const std::string wxExTokenizer::GetToken() const
{
  return m_Text.substr(
    m_TokenStartPos, 
    m_TokenEndPos != std::string::npos ? m_TokenEndPos - m_TokenStartPos: std::string::npos);
}
  
bool wxExTokenizer::HasMoreTokens() const
{
  return 
    !m_Delimiters.empty() && 
     m_TokenEndPos != std::string::npos &&
     (!m_SkipEmptyTokens || 
       m_Text.find_first_not_of(m_Delimiters, m_TokenEndPos) != std::string::npos);
}
