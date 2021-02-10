////////////////////////////////////////////////////////////////////////////////
// Name:      tokenizer.cpp
// Purpose:   Implementation of class wex::tokenizer
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/tokenizer.h>

wex::tokenizer::tokenizer(
  const std::string& text, const std::string& delimiters, bool skip_empty_tokens)
  : m_delimiters(delimiters)
  , m_skip_empty_tokens(skip_empty_tokens)
  , m_text(text)
{
}

size_t wex::tokenizer::count_tokens() const 
{
  size_t count = 0;

  for (tokenizer tkz(m_text, m_delimiters, m_skip_empty_tokens); tkz.has_more_tokens(); )
  {
    tkz.get_next_token();
    count++;
  }

  return count;
}

const std::string wex::tokenizer::get_next_token() 
{
  if (!has_more_tokens()) return std::string();

  if (m_skip_empty_tokens)
  {
    m_token_start_pos = m_text.find_first_not_of(m_delimiters, m_token_end_pos);
    m_start_pos = m_token_start_pos;
  }
  else
  {
    m_token_start_pos = m_start_pos;
  }
  
  if (m_token_start_pos == std::string::npos)
  {
    return std::string();
  }

  m_token_end_pos = m_text.find_first_of(m_delimiters, m_start_pos);
  m_last_delimiter = (m_token_end_pos != std::string::npos ? m_text[m_token_end_pos]: 0);

  if (!m_skip_empty_tokens)
  {
    m_start_pos = m_token_end_pos + 1;
  }

  return get_token();
}

const std::string wex::tokenizer::get_string() const
{
  if (m_token_end_pos == std::string::npos)
  {
    return std::string();
  }

  auto pos = m_token_end_pos;

  // skip leading delimiters
  while (pos < m_text.size() && m_delimiters.find(m_text[pos]) != std::string::npos)
  {
    pos++;
  }
  
  return m_text.substr(pos);
}
  
const std::string wex::tokenizer::get_token() const
{
  return m_text.substr(
    m_token_start_pos, 
    m_token_end_pos != std::string::npos ? m_token_end_pos - m_token_start_pos: std::string::npos);
}
  
bool wex::tokenizer::has_more_tokens() const
{
  return 
    !m_delimiters.empty() && 
     m_token_end_pos != std::string::npos &&
     (!m_skip_empty_tokens || 
       m_text.find_first_not_of(m_delimiters, m_token_end_pos) != std::string::npos);
}
