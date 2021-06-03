////////////////////////////////////////////////////////////////////////////////
// Name:      ex.h
// Purpose:   Declaration of class wex::ex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <wex/ex-command.h>
#include <wex/marker.h>

namespace wex
{
class ctags;
class ex_stream;
class macros;
class macro_mode;
class frame;

enum class info_message_t
{
  ADD,
  COPY,
  DEL,
  MOVE,
  NONE,
  YANK,
};

/// Offers a class that adds ex editor to wex::factory::stc.
class ex
{
  friend class macro_mode;

public:
  /// Returns the macros.
  static auto& get_macros() { return m_macros; }

  /// The visual modes.
  enum mode_t
  {
    OFF,    // off, not using ex or vi mode
    EX,     // ex mode, without vi keys, for reading large files
    VISUAL, // vi mode
  };

  /// Constructor.
  /// Provide stc cpomponent and ex mode.
  explicit ex(factory::stc* stc, mode_t mode = VISUAL);

  /// Destructor.
  virtual ~ex();

  /// Virtual interface.

  /// Executes ex: command that was entered on the command line,
  /// or present as modeline command inside a file.
  /// Returns true if the command was executed.
  virtual bool command(const std::string& command);

  /// Other methods.

  /// Returns calculated value of text.
  int calculator(const std::string& text);

  /// Copies data from other component.
  void copy(const ex* ex);

  /// Returns the ctags.
  auto& ctags() { return m_ctags; }

  /// Cuts selected text to yank register,
  /// and updates delete registers.
  void cut();

  /// The ex stream (used if in ex mode).
  auto       ex_stream() { return m_ex_stream; }
  const auto ex_stream() const { return m_ex_stream; }

  /// Returns the frame.
  auto* frame() { return m_frame; }

  /// Returns command.
  const auto& get_command() const { return m_command; }

  /// Returns stc component.
  factory::stc* get_stc() const;

  /// Returns whether ex is active.
  auto is_active() const { return m_mode != OFF; }

  /// Adds marker at the specified line.
  /// Returns true if marker could be added.
  bool marker_add(
    /// marker
    char marker,
    /// line to add marker, default current line
    int line = -1);

  /// Deletes specified marker.
  /// Returns true if marker was deleted.
  bool marker_delete(char marker);

  /// Goes to specified marker.
  /// Returns true if marker exists.
  bool marker_goto(char marker);

  /// Returns line for specified marker.
  /// Returns LINE_NUMBER_UNKNOWN if marker does not exist.
  int marker_line(char marker) const;

  /// Prints text in the dialog.
  void print(const std::string& text);

  /// Returns text to be inserted.
  const std::string register_insert() const;

  /// Returns current register name.
  auto register_name() const { return m_register; }

  /// Returns text from current register (or yank register if no register
  /// active).
  const std::string register_text() const;

  /// Resets search flags.
  void reset_search_flags();

  /// Returns search flags.
  auto search_flags() const { return m_search_flags; }

  /// Sets delete registers 1 - 9 (if value not empty).
  void set_registers_delete(const std::string& value) const;

  /// Sets insert register (if value not empty).
  void set_register_insert(const std::string& value) const;

  /// Sets yank register (if value not empty).
  void set_register_yank(const std::string& value) const;

  /// Set mode.
  void use(mode_t mode);

  /// Returns current visual mode.
  mode_t visual() const { return m_mode; }

  /// Yanks selected text to yank register, default to yank register.
  /// Returns false if no text was selected.
  bool yank(char name = '0') const;

protected:
  /// If autowrite is on and document is modified,
  /// save the document.
  bool auto_write();

  /// Sets register name.
  /// Setting register 0 results in
  /// disabling current register.
  void set_register(char name) { m_register = name; }

  ex_command m_command;

private:
  enum class address_t;

  bool address_parse(
    std::string& command,
    std::string& range,
    std::string& cmd,
    address_t&   type);
  bool command_address(const std::string& command);
  bool command_handle(const std::string& command) const;
  bool command_set(const std::string& command);

  template <typename S, typename T>
  bool handle_container(
    const std::string&                                          kind,
    const std::string&                                          command,
    const T*                                                    container,
    std::function<bool(const std::string&, const std::string&)> cb);

  void info_message(const std::string& text, info_message_t type) const;

  template <typename S, typename T>
  std::string report_container(const T& container) const;

  void show_dialog(
    const std::string& title,
    const std::string& text,
    const std::string& lexer = std::string());

  const marker m_marker_symbol = marker(0);
  const std::vector<std::pair<
    const std::string,
    std::function<bool(const std::string& command)>>>
    m_commands;

  static macros m_macros;

  bool m_auto_write{false}, m_copy{false}; // this is a copy, result of split

  int m_search_flags;

  char m_register{0};

  wex::ctags* m_ctags;
  wex::frame* m_frame;

  class ex_stream* m_ex_stream{nullptr};

  mode_t m_mode;

  std::map<char, int>
    // relate a marker to identifier
    m_marker_identifiers,
    // relate a marker to mark number
    m_marker_numbers;
};

/// Expands all markers and registers in command.
/// Returns false if an error occurred.
bool marker_and_register_expansion(const ex* ex, std::string& command);
}; // namespace wex
