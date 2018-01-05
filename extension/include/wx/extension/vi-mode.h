////////////////////////////////////////////////////////////////////////////////
// Name:      vi-mode.h
// Purpose:   Declaration of class wxExViFSMEntry and wxExViMode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <wx/dlimpexp.h>

class wxExVi;
class wxExViFSM;

/// The possible vi modes.
enum class wxExViModes
{
  NORMAL,      ///< normal (command or navigation) mode
  INSERT,      ///< pressing key inserts key
  INSERT_RECT, ///< as insert, while in visual rect mode
  VISUAL,      ///< navigation keys extend selection
  VISUAL_LINE, ///< complete lines are selected
  VISUAL_RECT, ///< navigation keys extend rectangular selection
};

/// Offers vi mode.
class WXDLLIMPEXP_BASE wxExViMode
{
public:  
  /// Constructor, 
  wxExViMode(
    /// specify vi component
    wxExVi* vi, 
    /// method to be called when going into insert mode
    std::function<void(const std::string& command)> insert = nullptr,
    /// method to be called when going back to normal mode
    std::function<void()> normal = nullptr);

  /// Destructor.
 ~wxExViMode();

  /// Escapes current mode.
  bool Escape() {return Transition("\x1b");};

  /// Returns the mode we are in.
  wxExViModes Get() const;
  
  /// Returns insert commands.
  const auto & GetInsertCommands() const {return m_InsertCommands;};

  /// Returns true if in insert mode.
  bool Insert() const {return 
    Get() == wxExViModes::INSERT || 
    Get() == wxExViModes::INSERT_RECT;};
  
  /// Returns true if in normal mode.
  bool Normal() const {return Get() == wxExViModes::NORMAL;};

  /// Returns mode as a string.
  const std::string String() const;
  
  /// Transitions to other mode depending on command.
  /// Returns true if command represents a mode change, otherwise false.
  /// If true is returned, it does not mean that mode was changed, in case 
  /// of readonly doc.
  bool Transition(const std::string& command);

  /// Returns true if in visual mode.
  bool Visual() const {return 
    Get() == wxExViModes::VISUAL || 
    Get() == wxExViModes::VISUAL_LINE || 
    Get() == wxExViModes::VISUAL_RECT;};
private:  
  wxExVi* m_vi;
  std::unique_ptr<wxExViFSM> m_FSM;
  const std::vector<std::pair<int, std::function<void()>>> m_InsertCommands;
};
