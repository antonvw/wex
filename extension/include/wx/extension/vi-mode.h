////////////////////////////////////////////////////////////////////////////////
// Name:      vi-mode.h
// Purpose:   Declaration of class wex::vifsmentry and wex::vi_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace wex
{
  class vi;
  class vifsm;

  /// The possible vi modes.
  enum class vi_modes
  {
    NORMAL,      ///< normal (command or navigation) mode
    INSERT,      ///< pressing key inserts key
    INSERT_RECT, ///< as insert, while in visual rect mode
    VISUAL,      ///< navigation keys extend selection
    VISUAL_LINE, ///< complete lines are selected
    VISUAL_RECT, ///< navigation keys extend rectangular selection
  };

  /// Offers vi mode.
  class vi_mode
  {
  public:  
    /// Constructor, 
    vi_mode(
      /// specify vi component
      vi* vi, 
      /// method to be called when going into insert mode
      std::function<void(const std::string& command)> insert = nullptr,
      /// method to be called when going back to normal mode
      std::function<void()> normal = nullptr);

    /// Destructor.
   ~vi_mode();

    /// Escapes current mode.
    bool Escape() {
      std::string command("\x1b");
      return Transition(command);};

    /// Returns the mode we are in.
    vi_modes Get() const;
    
    /// Returns insert commands.
    const auto & GetInsertCommands() const {return m_InsertCommands;};

    /// Returns true if in insert mode.
    bool Insert() const {return 
      Get() == vi_modes::INSERT || 
      Get() == vi_modes::INSERT_RECT;};
    
    /// Returns true if in normal mode.
    bool Normal() const {return Get() == vi_modes::NORMAL;};

    /// Returns mode as a string.
    const std::string String() const;
    
    /// Transitions to other mode depending on command.
    /// Returns true if command represents a mode change, otherwise false.
    /// If true is returned, it does not mean that mode was changed, in case 
    /// of readonly doc.
    bool Transition(std::string& command);

    /// Returns true if in visual mode.
    bool Visual() const {return 
      Get() == vi_modes::VISUAL || 
      Get() == vi_modes::VISUAL_LINE || 
      Get() == vi_modes::VISUAL_RECT;};
  private:  
    vi* m_vi;
    std::unique_ptr<vifsm> m_FSM;
    const std::vector<std::pair<int, std::function<void()>>> m_InsertCommands;
  };
};
