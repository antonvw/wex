////////////////////////////////////////////////////////////////////////////////
// Name:      shell.h
// Purpose:   Declaration of class wxExShell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <vector>
#include <wx/extension/stc.h>

#if wxUSE_GUI
class wxExProcess;

/// This class offers a wxExSTC with support for commands.
/// The commands are entered at the last line, and kept in a list of commands,
/// by pressing key up and down you browse through the commands.
/// If a command is entered, an ID_SHELL_COMMAND command event is sent to the
/// event handler, with the command available as event.GetString().
/// Or, if you used SetProcess, commands are sent to the process.
/// - If you press Ctrl-Q or Ctrl-C in the shell, and no text is selected,
///   a ID_SHELL_COMMAND_STOP is sent to the event handler.
/// - If you enter 'history', all previously entered commands are shown.
/// - If you enter !\<number\> the previous \<number\> command is entered.
/// - If you enter !\<abbreviation\> the last command starting with 
///   \<abbreviation\> is entered.
/// - Tab expansion is available for text entered matching existing files.
class WXDLLIMPEXP_BASE wxExShell: public wxExSTC
{
public:
  /// Constructor.
  wxExShell(
    /// Parent.
    wxWindow* parent,
    /// Give the command used to end a line.
    /// The default uses the GetEOL.
    const wxString& prompt = ">",
    /// The command used to end a line.
    const wxString& command_end = wxEmptyString,
    /// Will commands be echoed.
    bool echo = true,
    /// Give the number of commands that are kept in the config.
    /// Default -1, no commands are kept.
    int commands_save_in_config = -1,
    /// The lexer used by stc.
    const wxString& lexer = wxEmptyString,
    /// The stc menu flags.
    long menu_flags = STC_MENU_DEFAULT,
    /// The window id.
    wxWindowID id = wxID_ANY,
    /// Position.
    const wxPoint& pos = wxDefaultPosition,
    /// Size.
    const wxSize& size = wxDefaultSize,
    /// Window style.
    long style = 0);

  /// Destructor, keeps the commands in the config, if required.
 ~wxExShell();
 
  /// Appends text, and updates the command start position.
  /// Only if the cursor was at the end, the cursos is 
  /// repositioned at the end after appending the text, 
  void AppendText(const wxString& text);
 
  /// Enable/disable shell processing.
  /// Default (and after constructed) shell processing is enabled.
  /// When disabled, shell is a normal STC.
  void EnableShell(bool enable = true);
  
  /// Gets last entered command.
  const wxString GetCommand() const;
  
  /// Gets the command start position, that is the
  /// position after the prompt from where
  /// commands can be inserted.
  int GetCommandStartPosition() const {return m_CommandStartPosition;};
  
  /// Gets all history commands as a string, 
  /// separated by a newline (for testing).
  const wxString GetHistory() const;

  /// Gets the prompt.
  const wxString& GetPrompt() const {return m_Prompt;};

  /// Returns whether shell processing is enabled.
  bool GetShellEnabled() const {return m_Enabled;};
  
  // Paste the contents of the clipboard into the document replacing the selection.
  void Paste();
  
  /// Processes specified char.
  /// Returns true if char was processed.
  virtual bool ProcessChar(int c) override;
  
  /// Puts the text (if not empty) and a prompt at the end, goes to the end,
  /// and empties the undo buffer. 
  /// Default it also adds an eol before the prompt.
  /// Returns false and does not prompt if the shell is not enabled.
  bool Prompt(
    const wxString& text = wxEmptyString,
    bool add_eol = true);
    
  /// Sets the process to which commands are sent.
  /// If you do not set this, commands are sent to the parent.
  void SetProcess(wxExProcess* process);

  /// Sets the prompt, and prompts if asked for.
  /// Returns false and does not set the prompt if the shell is not enabled.
  bool SetPrompt(const wxString& prompt, bool do_prompt = true);
  
  /// Undo one action in the undo history.  
  virtual void Undo();
private:
  void Expand();
  void KeepCommand();
  void ProcessCharDefault(int key);
  /// Set command for command specified as number or as start of command,
  /// Returns true if found and m_Command was set.
  bool SetCommandFromHistory(const wxString& short_command);
  void ShowHistory();
  void ShowCommand(int key);

  // We use a list, as each command appears only once,
  // and when selecting an element already present,
  // it is moved to the end of the list.
  std::list < wxString > m_Commands;
  std::list < wxString >::const_iterator m_CommandsIterator;
  std::vector < wxString > m_AutoCompleteList;

  wxString m_Command;
  const wxString m_CommandEnd;
  int m_CommandStartPosition;
  const bool m_Echo;
  bool m_Enabled;
  const wxString m_CommandsInConfigDelimiter;
  const int m_CommandsSaveInConfig;
  wxString m_Prompt;
  
  wxExProcess* m_Process;
};
#endif // wxUSE_GUI
