/******************************************************************************\
* File:          shell.h
* Purpose:       Declaration of class exSTCShell
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXSTCSHELL_H
#define _EXSTCSHELL_H

#include <wx/extension/stc.h>
#include <list>

#if wxUSE_GUI
/// This class offers an exSTC with support for commands.
/// The commands are entered at the last line, and kept in a list of commands,
/// by pressing key up and down you browse through the commands.
/// If a command is entered, an ID_SHELL_COMMAND command event is sent to the
/// parent, with the command available as event.GetString().
/// - If you press Ctrl-Q in the shell, a ID_SHELL_COMMAND_STOP is sent to the parent.
/// - If you enter 'history', all previously entered commands are shown.
/// - If you enter !<number> the previous <number> command is entered.
/// - If you enter !<abbreviation> the last command starting with <abbreviation>
///   is entered.
class exSTCShell: public exSTC
{
public:
  /// Constructor.
  exSTCShell(
    wxWindow* parent,
    const wxString& prompt = ">",
    /// Give the command used to end a line.
    /// The default uses the GetEOL.
    const wxString& command_end = wxEmptyString,
    bool echo = true,
    /// Give the number of commands that are kept in the config.
    /// Default -1, no commands are kept.
    int commands_save_in_config = -1,
    long type = STC_MENU_SIMPLE | STC_MENU_FIND);

  /// Destructor, keeps the commands in the config, if required.
 ~exSTCShell();

  /// Gets the prompt.
  const wxString& GetPrompt() const {return m_Prompt;};

  /// Puts the text (if not empty) and a prompt at the end, goes to the end,
  /// and empties the undo buffer. Default it also adds an eol before the prompt.
  void Prompt(
    const wxString& text = wxEmptyString,
    bool add_eol = true);

  /// Sets the prompt, and prompts if asked for.
  void SetPrompt(const wxString& prompt, bool do_prompt = true) {
    m_Prompt = prompt;
    if (do_prompt) Prompt();};
protected:
  /// Override the default from exSTC.
  /// When you manipulate commands, this should not cause contents changed.
  virtual bool GetContentsChanged() {return false;};
  void OnChar(wxKeyEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
private:
  bool GetHistory(const wxString& short_command);
  void KeepCommand();
  void ShowHistory();
  void ShowCommand(int key);

  // We use a list, as each command appears only once,
  // and when selecting an element already present,
  // it is moved to the end of the list.
  std::list < wxString > m_Commands;
  std::list < wxString >::const_iterator m_CommandsIterator;

  wxString m_Command;
  const wxString m_CommandEnd;
  int m_CommandStartPosition;
  const bool m_Echo;
  const wxString m_CommandsInConfigDelimiter;
  const int m_CommandsSaveInConfig;
  wxString m_Prompt;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
