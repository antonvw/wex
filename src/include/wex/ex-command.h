////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.h
// Purpose:   Declaration of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
  class stc;

  /// Offers a command to be used by ex command.
  class ex_command
  {
  public:
    /// The type of ex command.
    enum class type_t
    {
      CALC,        ///< a calculation command (control-r =)
      COMMAND,     ///< a normal command (:)
      EXEC,        ///< an exec command (!)
      FIND,        ///< a find command (/ or ?)
      FIND_MARGIN, ///< a find command (/ or ?) entered on text margin
      NONE,        ///< an empty command 
      REPLACE,     ///< a replace command
      VI,          ///< a vi command (no ex command)
    };

    /// Default constructor.
    ex_command();

    /// Constructor, sets stc component.
    ex_command(stc* stc);

    /// Constructor, sets command text.
    ex_command(const std::string& text);

    /// Copy constructor.
    ex_command(const ex_command& c);

    /// Assignment operator.
    ex_command& operator=(const ex_command& c);

    /// Appends a char.  
    ex_command& append(char c) {
      m_text += std::string(1, c); return *this;};

    /// Appends a string.
    ex_command& append(const std::string& s) {m_text += s; return *this;};

    /// Appends a char and tries to execute.
    bool append_exec(char c);

    /// Returns last char of command.
    auto back() const {return m_text.back();};

    /// Clears command text. 
    void clear() {m_text.clear();};

    /// Returns the command text.
    const auto & command() const {return m_text;};

    /// Returns true if command text is empty.
    auto empty() const {return m_text.empty();};

    /// Executes the command on the STC component if available.
    bool exec();

    /// Returns front of command text.
    auto front() const {return m_text.front();};

    /// Returns stc component.
    auto * get_stc() const {return m_stc;};

    /// Removes last char of command text.
    void pop_back() {m_text.pop_back();};

    /// Restores values, if possible from original stc.
    void restore(const ex_command& c);
    
    /// Sets command text.
    /// The text should start with a command prefix,
    /// like ':' to return the command type.
    ex_command& set(const std::string& text);
    
    /// Sets new command, except original stc.
    /// This can be used in combination with restore.
    void set(const ex_command& c);

    /// Returns size of command.
    auto size() const {return m_text.size();};

    /// Returns type of command.
    type_t type() const;
  private:
    std::string m_text;
    wex::stc *m_stc {nullptr}, *m_stc_original {nullptr};
  };
};
