////////////////////////////////////////////////////////////////////////////////
// Name:      ex.h
// Purpose:   Declaration of class wex::ex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/regex.h>
#include <wex/core/type-to-value.h>
#include <wex/factory/ex-command.h>
#include <wex/factory/line-data.h>
#include <wex/syntax/lexer-props.h>
#include <wex/syntax/marker.h>

#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace wex
{
class addressrange;
class ctags;
class ex_stream;
class macros;
class macro_mode;
class frame;

namespace syntax
{
class stc;
};

/// The message shown for some action.
enum class info_message_t
{
  ADD,  ///< adding lines
  COPY, ///< copying lines
  DEL,  ///< erasing lines
  MOVE, ///< moving lines
  NONE, ///< no action shown
  YANK, ///< yanking lines
};

/// Offers a class that adds ex editor to wex::syntax::stc.
class ex
{
  friend class macro_mode;

public:
  // Static interface.

  /// Returns the macros.
  static auto& get_macros() { return m_macros; }

  /// Returns text to be inserted.
  static const std::string register_insert();

  /// Sets delete registers 1 - 9.
  static void set_registers_delete(const std::string& value);

  /// Sets insert register.
  static void set_register_insert(const std::string& value);

  /// Sets yank register.
  static void set_register_yank(const std::string& value);

  // Other methods.

  /// The visual modes.
  enum class mode_t
  {
    OFF,    ///< not using ex or vi mode
    EX,     ///< ex mode, without vi keys, for reading large files
    VISUAL, ///< normal vi mode
  };

  /// Constructor.
  /// Provide stc component and ex mode.
  explicit ex(syntax::stc* stc, mode_t mode = mode_t::VISUAL);

  /// Destructor.
  virtual ~ex();

  // Virtual interface.

  /// Executes ex: command that was entered on the command line,
  /// or present as modeline command inside a file.
  /// Returns true if the command was executed.
  virtual bool command(const std::string& command);

  // Other methods.

  /// Returns calculated value of text.
  std::optional<int> calculator(const std::string& text);

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

  /// Returns last printed text.
  const auto& get_print_text() const { return m_print_text; }

  /// Returns stc component.
  syntax::stc* get_stc() const;

  /// Shows info message.
  void info_message(const std::string& text, info_message_t type) const;

  /// Returns whether ex is active.
  auto is_active() const { return m_mode != mode_t::OFF; }

  /// Returns whether text specifies an address.
  bool is_address(const std::string& text);

  /// Returns line data.
  const auto& line_data() const { return m_data; }

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
  /// The command should start with backquote or `, followed
  /// by marker.
  bool marker_goto(const std::string& command);

  /// Returns line for specified marker.
  /// Returns LINE_NUMBER_UNKNOWN if marker does not exist.
  int marker_line(char marker) const;

  /// Prints address range.
  bool print(
    const addressrange& ar,
    const std::string&  flags     = std::string(),
    bool                separator = false);

  /// Prints text in the dialog.
  void print(const std::string& text);

  /// Returns text from current register (or yank register if no register
  /// active).
  const std::string register_text() const;

  /// Resets search flags, to what is available in config.
  void reset_search_flags();

  /// Returns search flags.
  auto search_flags() const { return m_search_flags; }

  /// Sets the whole word flag in search flags.
  void search_whole_word();

  /// Sets data.
  void set_line_data(const wex::line_data& data) { m_data = data; }

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

  /// Returns current register name.
  auto register_name() const { return m_register; }

  /// Sets register name.
  /// Setting register 0 results in
  /// disabling current register.
  void set_register(char name) { m_register = name; }

  ex_command  m_command;
  std::string m_command_string;

private:
  typedef std::vector<std::pair<
    const std::string,
    std::function<bool(const std::string& command)>>>
    commands_t;

  bool       command_handle(const std::string& command) const;
  bool       command_set(const std::string& command);
  commands_t commands_ex();

  template <typename S, typename T>
  bool handle_container(
    const std::string&                                          kind,
    const std::string&                                          command,
    const T*                                                    container,
    std::function<bool(const std::string&, const std::string&)> cb);

  template <typename S, typename T>
  std::string report_container(const T& container) const;

  void show_dialog(
    const std::string& title,
    const std::string& text,
    const std::string& lexer = std::string());

  const marker     m_marker_symbol = marker(0);
  const commands_t m_commands;

  static macros m_macros;

  bool m_auto_write{false}, m_copy{false}; // this is a copy, result of split

  const int m_search_flags_regex;
  int       m_search_flags;

  char m_register{0};

  wex::ctags*    m_ctags;
  wex::frame*    m_frame;
  wex::line_data m_data;

  class ex_stream* m_ex_stream{nullptr};

  mode_t m_mode;

  std::unordered_map<char, int>
    // relate a marker to column on a line
    m_marker_columns,
    // relate a marker to identifier
    m_marker_identifiers,
    // relate a marker to mark number
    m_marker_numbers;

  std::string m_print_text;
};

/// Expands all markers and registers in command.
/// Returns false if an error occurred.
bool marker_and_register_expansion(const ex* ex, std::string& command);
}; // namespace wex

// implementation

template <typename S, typename T>
bool wex::ex::handle_container(
  const std::string&                                          kind,
  const std::string&                                          command,
  const T*                                                    container,
  std::function<bool(const std::string&, const std::string&)> cb)
{
  // command is like:
  // :map 7 :%d
  if (regex v("(\\S+) +(\\S+) +(\\S+)"); v.match(command) == 3)
  {
    cb(v[1], v[2]);
  }
  else if (container != nullptr)
  {
    show_dialog(
      kind,
      report_container<S, T>(*container),
      lexer_props().scintilla_lexer());
  }

  return true;
}

template <typename S, typename T>
std::string wex::ex::report_container(const T& t) const
{
  const wex::lexer_props l;
  std::string            output;

  for (const auto& it : t)
  {
    output += l.make_key(type_to_value<S>(it.first).get_string(), it.second);
  }

  return output;
}
