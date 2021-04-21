////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream-line.cpp
// Purpose:   Implementation of class wex::ex_stream_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string.h>
#include <wex/addressrange.h>
#include <wex/data/substitute.h>
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
      ACTION_INSERT,
      ACTION_JOIN,
      ACTION_SUBSTITUTE,
      ACTION_WRITE,
    };

    /// Constructor for other action.
    ex_stream_line(action_t type, file* work, const addressrange& range);

    /// Constructor for substitute action.
    ex_stream_line(
      file*                   work,
      const addressrange&     range,
      const data::substitute& data);

    /// Constructor for insert action.
    ex_stream_line(
      file*               work,
      const addressrange& range,
      const std::string&  text);

    /// Destructor.
    ~ex_stream_line();

    /// Returns current action.
    auto action() const { return m_action; };

    /// Returns actions.
    int actions() const { return m_actions; };

    /// Handles a line.
    void handle(char* line, int& pos);

    /// Returns lines.
    int lines() const { return m_line; };

  private:
    const action_t         m_action;
    const data::substitute m_data;
    const std::string      m_text;
    file*                  m_file;
    const int              m_begin, m_end;
    int                    m_actions = 0, m_line = 0;
  };
}; // namespace wex
