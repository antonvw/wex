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
    vi(stc* stc);

    /// Appends string to executed insert command.
    void AppendInsertCommand(const std::string& s);
    
    /// Appends string to insert text.
    void AppendInsertText(const std::string& s);
    
    /// Executes vi command.
    /// Returns true if the command was executed.
    virtual bool Command(const std::string& command) override;
    
    /// Returns inserted text.
    const auto & GetInsertedText() const {return m_InsertText;};
    
    /// Returns last entered command.
    const auto & GetLastCommand() const {return m_LastCommand;};

    /// Returns motion commands.
    const auto & GetMotionCommands() const {return m_MotionCommands;};

    /// Returns other commands.
    const auto & GetOtherCommands() const {return m_OtherCommands;};

    /// Returns the mode we are in.  
    const auto & Mode() const {return m_Mode;};

    /// Returns writeable mode.  
    auto & Mode() {return m_Mode;};

    /// Handles char events.
    /// Returns true if event is allowed to be skipped.
    /// This means that the char is not handled by vi,
    /// e.g. vi mode is not active, or we are in insert mode,
    /// so the char should be handled by stc.
    bool OnChar(const wxKeyEvent& event);

    /// Handles keydown events.
    /// See OnChar.
    bool OnKeyDown(const wxKeyEvent& event);

    /// Extend visual selection.
    void VisualExtend(int start_pos, int end_pos);
  private:
    /// commands to be used in lambda
    typedef std::vector<std::pair<
      /// the command
      const std::string, 
      /// command callback, returns number of chars processed
      /// by this command
      std::function<size_t(const std::string& command)>>> commands;

    enum motion_type
    {
      MOTION_CHANGE,
      MOTION_DELETE,
      MOTION_NAVIGATE,
      MOTION_YANK,
    };

    void AddText(const std::string& text);
    void CommandCalc(const std::string& reg);
    void CommandReg(const char reg);
    void FilterCount(std::string& command);
    bool InsertMode(const std::string& text);
    void InsertModeNormal(const std::string& text);
    bool MotionCommand(motion_type type, std::string& command);
    bool OtherCommand(std::string& command);
    bool ParseCommand(std::string& command);
    bool Put(bool after);
    void SetLastCommand(const std::string& command);
    
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
