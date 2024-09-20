////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream-line.h
// Purpose:   Declaration of class wex::ex_stream_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unordered_map>
#include <wex/core/file.h>
#include <wex/core/reflection.h>
#include <wex/data/substitute.h>
#include <wex/ex/addressrange.h>

namespace wex
{
class ex_stream_line
{
public:
  enum action_t
  {
    ACTION_COPY,
    ACTION_ERASE,
    ACTION_GET,
    ACTION_INSERT,
    ACTION_JOIN,
    ACTION_MOVE,
    ACTION_SUBSTITUTE,
    ACTION_WRITE,
    ACTION_YANK,
  };

  enum handle_t
  {
    HANDLE_CONTINUE,
    HANDLE_ERROR,
    HANDLE_STOP,
  };

  /// Constructor for ACTION_INSERT action.
  ex_stream_line(
    file*               work,
    const addressrange& range,
    const std::string&  text);

  /// Constructor for ACTION_SUBSTITUTE action.
  ex_stream_line(
    file*                   work,
    const addressrange&     range,
    const data::substitute& data);

  /// Constructor for ACTION_MOVE or COPY action.
  ex_stream_line(
    file*               work,
    const addressrange& range,
    const address&      dest,
    action_t            type);

  /// Constructor for ACTION_YANK action.
  ex_stream_line(file* work, const addressrange& range, char name);

  /// Constructor for other actions.
  /// Used as delegate constructor.
  ex_stream_line(
    file*                   work,
    action_t                type,
    const addressrange&     range,
    const std::string&      text = std::string(),
    const data::substitute& data = data::substitute(),
    char                    name = 0,
    const address&          dest = address());

  /// Destructor.
  ~ex_stream_line();

  /// Returns current action.
  auto action() const { return m_action; }

  /// Returns actions.
  int actions() const { return m_actions; }

  /// Returns copy value.
  auto& copy() const { return m_copy; }

  /// Handles a line.
  handle_t handle(char* line, size_t& pos);

  /// Returns true if action is allowed to write.
  bool is_write() const
  {
    return m_action != ACTION_GET && m_action != ACTION_YANK;
  }

  /// Returns lines.
  int lines() const { return m_line; }

private:
  void handle_substitute(char* line, size_t pos);

  const action_t         m_action;
  const data::substitute m_data;
  const std::string      m_text;
  const char             m_register{0};
  const int              m_begin, m_end, m_dest{-1};

  file* m_file;
  int   m_actions{0}, m_line{0};

  std::string m_copy;

  reflection m_reflect;

  static const std::unordered_map<action_t, std::string> m_action_names;
};
}; // namespace wex
