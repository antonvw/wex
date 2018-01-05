////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.h
// Purpose:   Declaration of class wxExExCommand
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

class wxExSTC;

/// Offers a command to be used by ex Command.
class wxExExCommand
{
public:
  /// Default constructor, command should start with a ':'
  wxExExCommand(const std::string command = std::string());

  /// Constructor, supply STC component.
  wxExExCommand(wxExSTC* stc);

  /// Copy constructor.
  wxExExCommand(const wxExExCommand& c);

  /// Assignment operator.
  wxExExCommand& operator=(const wxExExCommand& c);

  /// Returns last char of command.
  auto back() const {return m_Command.back();};

  /// Clears data. 
  size_t clear();

  /// Returns ture if command is empty.
  auto empty() const {return m_Command.empty();};

  /// Returns front of command.
  auto front() const {return m_Command.front();};

  /// Removes last char of command.
  void pop_back() {m_Command.pop_back();};

  /// Returns size of command.
  auto size() const {return m_Command.size();};

  /// Appends a char to command.  
  wxExExCommand& Append(char c) {
    m_Command += std::string(1, c); return *this;};

  /// Appends a string to command.  
  wxExExCommand& Append(const std::string s) {m_Command += s; return *this;};

  /// Appends a char to command and tries to execute the command.  
  bool AppendExec(char c);

  /// Returns the command.
  auto & Command() {return m_Command;};

  /// Returns the command.
  const auto & Command() const {return m_Command;};

  /// Sets command.
  wxExExCommand& Command(const std::string& command) {
    m_Command = command; return *this;};

  /// Executes the command.
  /// If command is empty, use current command.
  bool Exec(const std::string& command = std::string());

  /// Returns is handled.
  bool IsHandled() const {return m_IsHandled;};

  /// Sets handled.
  wxExExCommand& IsHandled(bool is_handled) {
    m_IsHandled = is_handled; return *this;};

  /// Restores values, if possible from original stc.
  void Restore(const wxExExCommand& c);

  /// Sets new values, except original stc.
  /// This can be used in combination with Restore.
  void Set(const wxExExCommand& c);

  /// Returns STC component.
  auto * STC() const {return m_STC;};
private:
  std::string m_Command;
  bool m_IsHandled {false};
  wxExSTC *m_STC {nullptr}, *m_STC_original {nullptr};
};
