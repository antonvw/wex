////////////////////////////////////////////////////////////////////////////////
// Name:      shell.h
// Purpose:   Declaration of class wxExShell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <vector>
#include <wx/extension/stc.h>

class wxExProcess;

/// This class offers a wxExSTC with support for commands.
/// The commands are entered at the last line, and kept in a list of commands,
/// by pressing key up and down you browse through the commands.
/// If a command is entered, an ID_SHELL_COMMAND command event is sent to the
/// event handler, with the command available as event.GetString().
/// Or, if you used SetProcess, commands are sent to the process.
/// - If you press Ctrl-Q a ID_SHELL_COMMAND_STOP is sent to the event handler.
/// - If you enter 'history', all previously entered commands are shown.
/// - If you enter !\<number\> the previous \<number\> command is entered.
/// - If you enter !\<abbreviation\> the last command starting with 
///   \<abbreviation\> is entered.
/// - Tab expansion is available for text entered matching existing files.
class WXDLLIMPEXP_BASE wxExShell: public wxExSTC
{
public:
  /// Default constructor.
  wxExShell(
    /// data
    const wxExSTCData& data = wxExSTCData(),
    /// Give the command used to end a line.
    /// The default uses the GetEOL.
    const std::string& prompt = ">",
    /// The command used to end a line.
    const std::string& command_end = std::string(),
    /// Will commands be echoed.
    bool echo = true,
    /// The lexer used by stc.
    const std::string& lexer = std::string(),
    /// Give the number of commands that are kept in the config.
    /// If -1, no commands are kept.
    int commands_save_in_config = 100);

  /// Destructor, keeps the commands in the config, if required.
  virtual ~wxExShell();
 
  /// Appends text, and updates the command start position.
  /// Only if the cursor was at the end, the cursos is 
  /// repositioned at the end after appending the text, 
  virtual void AppendText(const wxString& text) override;
 
  /// Enable/disable shell processing.
  /// Default (and after constructed) shell processing is enabled.
  /// When disabled, shell is a normal STC.
  void EnableShell(bool enable = true);
  
  /// Returns last entered command.
  const std::string GetCommand() const;
  
  /// Returns all history commands as a string, 
  /// separated by a newline (for testing).
  const std::string GetHistory() const;

  /// Returns the prompt.
  const auto& GetPrompt() const {return m_Prompt;};

  /// Returns whether shell processing is enabled.
  bool GetShellEnabled() const {return m_Enabled;};
  
  // Paste the contents of the clipboard into the document replacing the selection.
  virtual void Paste() override;
  
  /// Processes specified char.
  /// Returns true if char was processed.
  virtual bool ProcessChar(int c) override;
  
  /// Puts the text (if not empty) and a prompt at the end, goes to the end,
  /// and empties the undo buffer. 
  /// Default it also adds an eol before the prompt.
  /// Returns false and does not prompt if the shell is not enabled.
  bool Prompt(
    const std::string& text = std::string(),
    bool add_eol = true);
  
  /// Sets the process to which commands are sent.
  /// If you do not set this, commands are sent to the parent.
  void SetProcess(wxExProcess* process);

  /// Sets the prompt, and prompts if asked for.
  /// Returns false and does not set the prompt if the shell is not enabled.
  bool SetPrompt(const std::string& prompt, bool do_prompt = true);
  
  /// Undo one action in the undo history.  
  virtual void Undo() override;
private:
  void Expand();
  void KeepCommand();
  void ProcessCharDefault(int key);
  /// Set command for command specified as number or as start of command,
  /// Returns true if found and m_Command was set.
  bool SetCommandFromHistory(const std::string& short_command);
  void ShowHistory();
  void ShowCommand(int key);

  const std::string m_CommandEnd;
  const bool m_Echo;
  const int m_CommandsSaveInConfig;

  // We use a list, as each command appears only once,
  // and when selecting an element already present,
  // it is moved to the end of the list.
  std::list < std::string > m_Commands;
  std::list < std::string >::const_iterator m_CommandsIterator;
  std::vector < std::string > m_AutoCompleteList;

  std::string m_Command;
  std::string m_Prompt;
  int m_CommandStartPosition = 0; /// position after the prompt from where commands can be inserted
  bool m_Enabled = true;
  
  wxExProcess* m_Process = nullptr;
};
