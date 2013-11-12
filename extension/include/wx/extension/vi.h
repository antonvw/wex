////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVI_H
#define _EXVI_H

#include <wx/extension/ex.h>

#if wxUSE_GUI

class wxExSTCEntryDialog;

/// Offers a class that extends wxExSTC with vi behaviour.
class WXDLLIMPEXP_BASE wxExVi : public wxExEx
{
public:
  /// The possible vi modes.
  enum wxExViMode
  {
    MODE_NORMAL,      /// normal, command or navigation, mode
    MODE_INSERT,      /// pressing key inserts key
    MODE_VISUAL,      /// navigation keys extend selection
    MODE_VISUAL_LINE, /// complete lines are selected
  };

  /// Constructor.
  wxExVi(wxExSTC* stc);
  
  /// Executes command in command mode (like 'j', or 'y').
  /// Returns true if the command was executed.
  virtual bool Command(const wxString& command);
  
  /// Returns text to be inserted.
  const wxString& GetInsertText() const {return m_InsertText;};
  
  /// Returns the mode we are in.
  int GetMode() const {return m_Mode;};
  
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
  void AddText(const wxString& text);
  bool ChangeNumber(bool inc);
  void CommandCalc(const wxString& reg);
  bool CommandChar(int c, int repeat);
  void CommandReg(const wxString& reg);
  void FindWord(bool find_next = true);
  void GotoBrace();
  bool Indent(
    const wxString& begin_address, 
    const wxString& end_address, 
    bool forward);
  bool InsertMode(const wxString& command);
  /// Adds recording to current macro.
  virtual void MacroRecord(const wxString& text);
  bool Put(bool after);
  bool SetInsertMode(
    const wxString& command,
    int repeat = 1);
  bool ToggleCase(); 
  void VisualExtendLeftLine();
  void VisualExtendRightLine();
  bool YankedLines();   

  static wxExSTCEntryDialog* m_Dialog;
  static wxString m_LastFindCharCommand;

  bool m_Dot;  
  bool m_SearchForward;
  
  int m_InsertRepeatCount;
  int m_Mode;
  
  wxString m_Command;
  wxString m_InsertText;
};
#endif // wxUSE_GUI
#endif
