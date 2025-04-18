////////////////////////////////////////////////////////////////////////////////
// Name:      path-match.cpp
// Purpose:   Declaration of class wex::path_match
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/path-match.h>
#include <wex/core/config.h>
#include <wx/wx.h>

wex::path_match::path_match(const wex::path& p)
  : m_path(p)
{
  ;
}

wex::path_match::path_match(
  const wex::path&   p,
  const wex::tool&   t,
  const std::string& line,
  size_t             line_no,
  int                pos)
  : m_line(line)
  , m_path(p)
  , m_pos(pos)
  , m_line_no(line_no)
  , m_tool(t)
{
  ;
};

const std::string wex::path_match::context() const
{
  const int context_size = config(_("list.Context size")).get(10);

  if (m_pos == -1 || context_size <= 0 || context_size >= m_line.size())
  {
    return m_line;
  }

  return (context_size > m_pos ? std::string(context_size - m_pos, ' ') :
                                 std::string()) +
         m_line.substr(context_size < m_pos ? m_pos - context_size : 0);
}
