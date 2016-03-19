////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
  enum wxExViMode
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
  bool ModeInsert() const {return GetMode() == MODE_INSERT || GetMode() == MODE_INSERT_RECT;};
  bool ModeNormal() const {return GetMode() == MODE_NORMAL;};
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
private:
  void AddText(const std::string& text);
  void CommandCalc(const wxString& reg);
  bool CommandChar(std::string& command);
  void CommandReg(const char reg);
  void FilterCount(std::string& command, const std::string& prefix = "");
  bool InsertMode(const std::string& text);
  void InsertModeNormal(const std::string& text);
  virtual void MacroRecord(const std::string& text) override;
  bool MotionCommand(int type, std::string& command, bool is_handled = false);
  bool OtherCommand(std::string& command) const;
  bool Put(bool after);

  static std::string m_LastFindCharCommand;

  bool m_Dot;  
  bool m_SearchForward;
  
  int m_Count;
  int m_Start;
  int m_Type;
  
  wxExViFSM m_FSM;
  
  std::string m_Command;
  std::string m_InsertText;

  const std::vector<std::pair<
    const std::string, 
    std::function<bool(const std::string& command)>>> m_MotionCommands;
  const std::vector<std::pair<
    const std::string, 
    std::function<bool(const std::string& command)>>> m_OtherCommands;
};
#endif // wxUSE_GUI
