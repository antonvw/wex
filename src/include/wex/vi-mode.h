////////////////////////////////////////////////////////////////////////////////
// Name:      vi-mode.h
// Purpose:   Declaration of class wex::vi_mode
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
  class vi_fsm;

  /// Offers vi mode.
  class vi_mode
  {
  public:  
    /// The possible vi mode states.
    enum state_t
    {
      NORMAL,      ///< normal (command or navigation) mode
      INSERT,      ///< pressing key inserts key
      INSERT_RECT, ///< as insert, while in visual rect mode
      VISUAL,      ///< navigation keys extend selection
      VISUAL_LINE, ///< complete lines are selected
      VISUAL_RECT, ///< navigation keys extend rectangular selection
    };

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

    /// escapes current mode.
    bool escape() {
      std::string command("\x1b");
      return transition(command);};

    /// Returns the state we are in.
    state_t get() const;
    
    /// Returns true if in insert mode.
    bool insert() const {return 
      get() == INSERT || 
      get() == INSERT_RECT;};
    
    /// Returns insert commands.
    const auto & insert_commands() const {return m_InsertCommands;};

    /// Returns true if in normal mode.
    bool normal() const {return get() == NORMAL;};

    /// Returns mode as a string.
    const std::string string() const;
    
    /// transitions to other mode depending on command.
    /// Returns true if command represents a mode change, otherwise false.
    /// If true is returned, it does not mean that mode was changed, in case 
    /// of readonly doc.
    bool transition(std::string& command);

    /// Returns true if in visual mode.
    bool visual() const {return 
      get() == VISUAL || 
      get() == VISUAL_LINE || 
      get() == VISUAL_RECT;};
  private:  
    vi* m_vi;
    std::unique_ptr<vi_fsm> m_FSM;
    const std::vector<std::pair<int, std::function<void()>>> m_InsertCommands;
  };
};
