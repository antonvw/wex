////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

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
    // all visual modes after this one
    MODE_VISUAL,      ///< navigation keys extend selection
    MODE_VISUAL_LINE, ///< complete lines are selected
    MODE_VISUAL_RECT, ///< navigation keys extend rectangular selection
  };

  /// Constructor.
  wxExVi(wxExSTC* stc);
  
  /// Executes vi command.
  /// Returns true if the command was executed.
  virtual bool Command(const std::string& command);
  
  /// Returns inserted text.
  const std::string& GetInsertedText() const {return m_InsertText;};
  
  /// Returns the mode we are in.
  int GetMode() const {return m_FSM.State();};
  int ModeInsert() const {return GetMode() == MODE_INSERT || GetMode() == MODE_INSERT_RECT;};
  int ModeNormal() const {return GetMode() == MODE_NORMAL;};
  int ModeVisual() const {return GetMode() >= MODE_VISUAL;};
  
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
  bool ChangeNumber(bool inc);
  void CommandCalc(const wxString& reg);
  bool CommandChar(int c);
  bool CommandChars(std::string& rest);
  void CommandReg(const char reg);
  bool FindChar(const wxString& text, const wxString& start);
  void FindWord(bool find_next = true);
  void GotoBrace();
  bool Indent(
    const wxString& begin_address, 
    const wxString& end_address, 
    bool forward);
  bool InsertMode(const std::string& text);
  void InsertModeNormal(const std::string& text);
  /// Adds recording to current macro.
  virtual void MacroRecord(const std::string& text);
  bool Put(bool after);
  bool ToggleCase(); 

  static std::string m_LastFindCharCommand;

  bool m_Dot;  
  bool m_SearchForward;
  
  int m_Repeat;
  
  wxExViFSM m_FSM;
  
  std::string m_Command;
  std::string m_InsertText;
};
#endif // wxUSE_GUI
