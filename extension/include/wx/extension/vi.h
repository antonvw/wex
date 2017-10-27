////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <wx/event.h>
#include <wx/extension/ex.h>
#include <wx/extension/vifsm.h>

#if wxUSE_GUI

/// Offers a class that extends wxExSTC with vi behaviour.
class WXDLLIMPEXP_BASE wxExVi : public wxExEx
{
public:
  /// The possible vi modes.
  enum Mode
  {
    MODE_NORMAL,      ///< normal (command or navigation) mode
    MODE_INSERT,      ///< pressing key inserts key
    MODE_INSERT_RECT, ///< as insert, while in visual rect mode
    MODE_VISUAL,      ///< navigation keys extend selection
    MODE_VISUAL_LINE, ///< complete lines are selected
    MODE_VISUAL_RECT, ///< navigation keys extend rectangular selection
  };

  /// Constructor.
  wxExVi(wxExSTC* stc);
  
  /// Executes vi command.
  /// Returns true if the command was executed.
  virtual bool Command(
    const std::string& command,
    bool is_handled = false) override;
  
  /// Returns insert commands.
  const auto & GetInsertCommands() const {return m_FSM.GetInsertCommands();};

  /// Returns inserted text.
  const auto & GetInsertedText() const {return m_InsertText;};
  
  /// Returns motion commands.
  const auto & GetMotionCommands() const {return m_MotionCommands;};

  /// Returns other commands.
  const auto & GetOtherCommands() const {return m_OtherCommands;};

  /// Returns the mode we are in.
  int GetMode() const {return m_FSM.State();};
  
  /// Returns the mode we are in as a string.
  const auto GetModeString() const {return m_FSM.StateString();};
  
  /// Returns true if in insert mode.
  bool ModeInsert() const {return GetMode() == MODE_INSERT || GetMode() == MODE_INSERT_RECT;};
  
  /// Returns true if in normal mode.
  bool ModeNormal() const {return GetMode() == MODE_NORMAL;};

  /// Returns true if in visual mode.
  bool ModeVisual() const {return GetMode() >= MODE_INSERT_RECT;};
  
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
    std::function<size_t(const std::string& command)>>> Commands;

  void AddText(const std::string& text);
  void CommandCalc(const std::string& reg);
  void CommandReg(const char reg);
  void FilterCount(std::string& command, const std::string& prefix = "");
  bool InsertMode(const std::string& text);
  void InsertModeNormal(const std::string& text);
  bool MotionCommand(int type, std::string& command, bool is_handled = false);
  bool OtherCommand(std::string& command) const;
  bool ParseCommand(std::string& command, bool is_handled);
  bool Put(bool after);
  bool TransitionCommand(std::string& command);

  static std::string m_LastFindCharCommand;
  bool m_Dot{false}, m_SearchForward{true};
  int m_Count{1};
  wxExViFSM m_FSM;
  std::string m_Command, m_CommandKeep, m_InsertText;
  const Commands m_MotionCommands, m_OtherCommands;
};
#endif // wxUSE_GUI
