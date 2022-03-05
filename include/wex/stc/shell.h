////////////////////////////////////////////////////////////////////////////////
// Name:      shell.h
// Purpose:   Declaration of class wex::shell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stc/stc.h>

#include <list>
#include <vector>

namespace wex
{
namespace factory
{
class process;
}

/// This class offers a stc with support for commands.
/// The commands are entered at the last line, and kept in a list of commands,
/// by pressing key up and down you browse through the commands.
/// If a command is entered, an ID_SHELL_COMMAND command event is sent to the
/// event handler, with the command available as event.GetString().
/// Or, if you used set_process, commands are sent to the process.
/// - If you press Ctrl-Q a ID_SHELL_COMMAND_STOP is sent to the event
///   handler.
/// - If you enter 'history', all previously entered commands are shown.
/// - If you enter !\<number\> the previous \<number\> command is entered.
/// - If you enter !\<abbreviation\> the last command starting with
///   \<abbreviation\> is entered.
/// - Tab expansion is available for text entered matching existing files.
class shell : public stc
{
public:
  /// Default constructor.
  shell(
    /// data
    const data::stc& data = data::stc(),
    /// Give the command used to end a line.
    /// The default uses the eol.
    const std::string& prompt = ">",
    /// The command used to end a line.
    const std::string& command_end = std::string());

  /// Destructor, keeps the commands in the config, if required.
  ~shell() override;

  /// Virtual interface

  /// Appends text, and updates the command start position.
  /// Only if the cursor was at the end, the cursor is
  /// repositioned at the end after appending the text,
  void AppendText(const wxString& text) override;

  // Paste the contents of the clipboard into the document replacing the
  // selection.
  void Paste() override;

  /// Undo one action in the undo history.
  void Undo() override;

  /// Processes specified char.
  /// Returns true if char was processed.
  bool process_char(int c) override;

  /// Other methods

  /// Enable/disable shell processing.
  /// Default (and after constructed) shell processing is enabled.
  /// When disabled, shell is a normal stc.
  void enable(bool enable = true);

  /// Returns last entered command.
  const std::string get_command() const;

  /// Returns all history commands as a string,
  /// separated by a newline (for testing).
  const std::string get_history() const;

  /// Returns the prompt.
  const auto& get_prompt() const { return m_prompt; }

  /// Returns whether shell processing is enabled.
  bool is_enabled() const { return m_enabled; }

  /// Puts the text (if not empty) and a prompt at the end, goes to the end,
  /// and empties the undo buffer.
  /// Default it also adds an eol before the prompt.
  /// Returns false and does not prompt if the shell is not enabled.
  bool prompt(const std::string& text = std::string(), bool add_eol = true);

  /// Sets the process to which commands are sent.
  /// If you do not set this, commands are sent to the parent.
  void set_process(factory::process* process);

  /// Sets the prompt, and prompts if asked for.
  /// Returns false and does not set the prompt if the shell is not enabled.
  bool set_prompt(const std::string& prompt, bool do_prompt = true);

private:
  void bind_other();
  void expand();
  void keep_command();

  void on_key_back_delete(wxKeyEvent& event);
  void on_key_down(wxKeyEvent& event);
  bool on_key_down_continue(wxKeyEvent& event);
  void on_key_down_others(wxKeyEvent& event);
  void on_key_home();
  void on_key_up_down(wxKeyEvent& event);

  void on_mouse(wxMouseEvent& event);
  void process_char_default(int key);
  void send_command();
  /// Set command for command specified as number or as start of command,
  /// Returns true if found and m_command was set.
  bool set_command_from_history(const std::string& short_command);
  void show_command(int key);
  void show_history();

  const std::string m_command_end;
  const bool        m_echo;
  const size_t      m_commands_save_in_config;

  // We use a list, as each command appears only once,
  // and when selecting an element already present,
  // it is moved to the end of the list.
  std::list<std::string>                 m_commands;
  std::list<std::string>::const_iterator m_commands_iterator;
  std::vector<std::string>               m_auto_complete_list;

  std::string m_command, m_prompt;

  int m_command_start_pos =
    0; /// position after the prompt from where commands can be inserted
  bool m_enabled = true;

  factory::process* m_process{nullptr};
};
}; // namespace wex
