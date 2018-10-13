////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.h
// Purpose:   Declaration of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
  /// The type of ex command.
  enum class ex_command_type
  {
    CALC,        ///< a calculation command (control R=)
    COMMAND,     ///< a normal command (:)
    EXEC,        ///< an exec command (:)
    FIND,        ///< a find command (/ or ?)
    FIND_MARGIN, ///< a find command (/ or ?) entered on text margin
    NONE,        ///< an empty command 
    REPLACE,     ///< a replace command
    VI,          ///< a vi command (no ex command)
  };

  class stc;

  /// Offers a command to be used by ex Command.
  class ex_command
  {
  public:
    /// Default constructor, command should start with a ':'
    ex_command(const std::string& command = std::string());

    /// Constructor, supply STC component.
    ex_command(stc* stc);

    /// Copy constructor.
    ex_command(const ex_command& c);

    /// Assignment operator.
    ex_command& operator=(const ex_command& c);

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
    ex_command& Append(char c) {
      m_Command += std::string(1, c); return *this;};

    /// Appends a string to command.  
    ex_command& Append(const std::string& s) {m_Command += s; return *this;};

    /// Appends a char to command and tries to execute the command.  
    bool AppendExec(char c);

    /// Returns the command.
    auto & Command() {return m_Command;};

    /// Returns the command.
    const auto & Command() const {return m_Command;};

    /// Sets command.
    ex_command& Command(const std::string& command) {
      m_Command = command; return *this;};

    /// Executes the command.
    /// If command is empty, use current command.
    bool Exec(const std::string& command = std::string());

    /// Returns is handled.
    bool IsHandled() const {return m_IsHandled;};

    /// Sets handled.
    ex_command& IsHandled(bool is_handled) {
      m_IsHandled = is_handled; return *this;};

    /// Restores values, if possible from original stc.
    void Restore(const ex_command& c);

    /// Sets new values, except original stc.
    /// This can be used in combination with Restore.
    void Set(const ex_command& c);

    /// Returns STC component.
    auto * STC() const {return m_STC;};

    /// Returns type of command.
    ex_command_type Type() const;
  private:
    std::string m_Command;
    bool m_IsHandled {false};
    stc *m_STC {nullptr}, *m_STC_original {nullptr};
  };
};
