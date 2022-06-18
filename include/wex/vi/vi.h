////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wex::vi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/vi/ex.h>
#include <wex/vi/mode.h>
#include <wx/event.h>

#include <functional>
#include <vector>

namespace wex
{
/// Offers a class that extends factory::stc with vi behaviour.
class vi : public ex
{
public:
  /// function type.
  typedef std::function<size_t(const std::string& command)> function_t;

  /// commands to be used in lambda
  typedef std::vector<std::pair<
    /// the command
    const std::string,
    /// command callback, returns number of chars processed
    /// by this command
    function_t>>
    commands_t;

  enum class motion_t;

  /// Constructor.
  explicit vi(wex::factory::stc* stc, mode_t mode = VISUAL);

  /// Appends string to executed insert command.
  void append_insert_command(const std::string& s);

  /// Appends string to insert text.
  void append_insert_text(const std::string& s);

  /// Executes vi command.
  /// Returns true if the command was executed.
  bool command(const std::string& command) final;

  /// Finishes vi command.
  bool command_finish();

  /// Returns inserted text.
  const auto& inserted_text() const { return m_insert_text; }

  /// Returns last entered command.
  const auto& last_command() const { return m_last_command; }

  /// Returns the mode we are in.
  const auto& mode() const { return m_mode; }

  /// Returns writeable mode.
  auto& mode() { return m_mode; }

  /// Returns motion commands.
  const auto& motion_commands() const { return m_motion_commands; }

  /// Handles char events.
  /// Returns true if event is allowed to be skipped.
  /// This means that the char is not handled by vi,
  /// e.g. vi mode is not active, or we are in insert mode,
  /// so the char should be handled by stc.
  bool on_char(const wxKeyEvent& event);

  /// Handles keydown events.
  /// See on_char.
  bool on_key_down(const wxKeyEvent& event);

  /// Returns other commands.
  const auto& other_commands() const { return m_other_commands; }

  /// Extend visual selection.
  void visual_extend(int start_pos, int end_pos) const;

private:
  void       command_reg(const std::string& reg);
  commands_t commands_motion();
  commands_t commands_other();

  char convert_key_event(const wxKeyEvent& event) const;
  bool delete_range(int start, int end);
  void filter_count(std::string& command);

  size_t find_char(const std::string& command);
  size_t find_command(const std::string& command);
  size_t find_next(const std::string& direction);
  size_t inc_or_dec(const std::string& command);

  motion_t get_motion(const std::string& command) const;

  bool insert_mode(const std::string& text);
  void insert_mode_escape(const std::string& command);
  bool insert_mode_hex(const std::string& command);
  void insert_mode_normal(const std::string& text);
  bool insert_mode_other(const std::string& text);
  bool insert_mode_register(const std::string& text);

  bool motion_command(motion_t type, std::string& command);
  bool motion_command_handle(motion_t type, std::string& command, function_t t);
  bool other_command(std::string& command);
  bool parse_command(std::string& command);
  bool parse_command_handle(const std::string& org, std::string& command);
  bool parse_command_handle_single(
    motion_t     type,
    std::string& command,
    bool&        check_other);
  bool
  parse_command_motion(motion_t type, std::string& command, bool& check_other);

  bool   process_macro_key(const wxKeyEvent& event);
  bool   process_special_key(const wxKeyEvent& event);
  bool   put(bool after);
  size_t reverse_case(const std::string& command);
  void   set_last_command(const std::string& command);
  void   yank_range(int start);

  static inline std::string m_last_command;
  static inline std::string m_last_find_char_command;

  bool m_count_present{false}, m_dot{false}, m_search_forward{true};

  int m_count{1};

  std::string m_insert_command, m_insert_text;

  vi_mode m_mode;

  const commands_t m_motion_commands, m_other_commands;

  const std::vector<std::string> m_last_commands;
};
}; // namespace wex
