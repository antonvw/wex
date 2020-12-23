////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream-line.cpp
// Purpose:   Implementation of class wex::ex_stream_lne
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string.h>
#include <wex/addressrange.h>
#include <wex/ex-stream.h>
#include <wex/file.h>

namespace wex
{
  class ex_stream_line
  {
  public:
    enum action_t
    {
      ACTION_ERASE,
      ACTION_JOIN,
      ACTION_SUBSTITUTE,
    };

    /// Constructor.
    ex_stream_line(action_t type, const addressrange& range, file* work);

    /// Constructor for substitute.
    ex_stream_line(
      const addressrange& range,
      file*               work,
      const std::string&  find,
      const std::string&  replace);

    /// Returns actions.
    int actions() const { return m_actions; };

    /// Handles a line.
    void handle(char* line, int& pos);

    /// Returns lines.
    int lines() const { return m_line; };

  private:
    file*               m_file;
    action_t            m_action;
    int                 m_actions = 0, m_line = 0;
    const addressrange& m_range;
    const std::string   m_find, m_replace;
  };
}; // namespace wex
