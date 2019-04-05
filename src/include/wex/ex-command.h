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
  class stc;

  /// Offers a command to be used by ex Command.
  class ex_command
  {
  public:
    /// The type of ex command.
    enum class type_t
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

    /// Default constructor, command should start with a ':'
    ex_command(const std::string& command = std::string());

    /// Constructor, supply stc component.
    ex_command(stc* stc);

    /// Copy constructor.
    ex_command(const ex_command& c);

    /// Assignment operator.
    ex_command& operator=(const ex_command& c);

    /// Appends a char to command.  
    ex_command& append(char c) {
      m_Command += std::string(1, c); return *this;};

    /// Appends a string to command.  
    ex_command& append(const std::string& s) {m_Command += s; return *this;};

    /// Appends a char to command and tries to execute the command.  
    bool append_exec(char c);

    /// Returns last char of command.
    auto back() const {return m_Command.back();};

    /// Clears data. 
    size_t clear();

    /// Returns the command.
    auto & command() {return m_Command;};

    /// Returns the command.
    const auto & command() const {return m_Command;};

    /// Sets command.
    ex_command& command(const std::string& command) {
      m_Command = command; return *this;};

    /// Returns ture if command is empty.
    auto empty() const {return m_Command.empty();};

    /// Executes the command.
    /// If command is empty, use current command.
    bool exec(const std::string& command = std::string());

    /// Returns front of command.
    auto front() const {return m_Command.front();};

    /// Returns stc component.
    auto * get_stc() const {return m_STC;};

    /// Returns is handled.
    bool is_handled() const {return m_is_handled;};

    /// Sets handled.
    ex_command& is_handled(bool is_handled) {
      m_is_handled = is_handled; return *this;};

    /// Removes last char of command.
    void pop_back() {m_Command.pop_back();};

    /// Restores values, if possible from original stc.
    void restore(const ex_command& c);

    /// Sets new values, except original stc.
    /// This can be used in combination with Restore.
    void set(const ex_command& c);

    /// Returns size of command.
    auto size() const {return m_Command.size();};

    /// Returns type of command.
    type_t type() const;
  private:
    std::string m_Command;
    bool m_is_handled {false};
    wex::stc *m_STC {nullptr}, *m_STC_original {nullptr};
  };
};
