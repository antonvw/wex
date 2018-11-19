////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wex::vi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <wx/event.h>
#include <wex/ex.h>
#include <wex/vi-mode.h>

namespace wex
{
  /// Offers a class that extends stc with vi behaviour.
  class vi : public ex
  {
  public:
    /// Constructor.
    vi(wex::stc* stc);

    /// Appends string to executed insert command.
    void append_insert_command(const std::string& s);
    
    /// Appends string to insert text.
    void append_insert_text(const std::string& s);
    
    /// Executes vi command.
    /// Returns true if the command was executed.
    virtual bool command(const std::string& command) override;
    
    /// Returns inserted text.
    const auto & inserted_text() const {return m_InsertText;};
    
    /// Returns last entered command.
    const auto & last_command() const {return m_LastCommand;};

    /// Returns the mode we are in.  
    const auto & mode() const {return m_Mode;};

    /// Returns writeable mode.  
    auto & mode() {return m_Mode;};

    /// Returns motion commands.
    const auto & motion_commands() const {return m_MotionCommands;};

    /// Handles char events.
    /// Returns true if event is allowed to be skipped.
    /// This means that the char is not handled by vi,
    /// e.g. vi mode is not active, or we are in insert mode,
    /// so the char should be handled by stc.
    bool on_char(const wxKeyEvent& event);

    /// Handles keydown events.
    /// See on_char.
    bool on_key_down(const wxKeyEvent& event);

    /// Returns other commands.
    const auto & other_commands() const {return m_OtherCommands;};

    /// Extend visual selection.
    void visual_extend(int start_pos, int end_pos);
  private:
    /// commands to be used in lambda
    typedef std::vector<std::pair<
      /// the command
      const std::string, 
      /// command callback, returns number of chars processed
      /// by this command
      std::function<size_t(const std::string& command)>>> commands;

    enum motion_t
    {
      MOTION_CHANGE,
      MOTION_DELETE,
      MOTION_NAVIGATE,
      MOTION_YANK,
    };

    void add_text(const std::string& text);
    void command_calc(const std::string& reg);
    void command_reg(const char reg);
    void filter_count(std::string& command);
    bool insert_mode(const std::string& text);
    void insert_mode_normal(const std::string& text);
    bool motion_command(motion_t type, std::string& command);
    bool other_command(std::string& command);
    bool parse_command(std::string& command);
    bool put(bool after);
    void set_last_command(const std::string& command);
    
    static inline std::string m_LastCommand;
    static inline std::string m_LastFindCharCommand;

    bool m_Dot{false}, m_SearchForward{true};
    int m_Count{1};
    std::string m_InsertCommand, m_InsertText;
    vi_mode m_Mode;
    const commands m_MotionCommands, m_OtherCommands;
    const std::vector<std::string> m_LastCommands;
  };
};
