////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Created:   2009-11-21
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVI_H
#define _EXVI_H

#include <map>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#if wxUSE_GUI

class wxExSTC;

/// Offers a class that extends STC with vi behaviour.
class wxExVi
{
public:
  /// Constructor.
  wxExVi(wxExSTC* stc);

  /// Handle char events.
  /// Returns true if event is allowed to be skipped.
  bool OnChar(wxKeyEvent& event);

  /// Handle keydown events.
  /// Returns true if event is allowed to be skipped.
  bool OnKeyDown(wxKeyEvent& event);
private:
  void Delete(int lines) const;
  void Delete(
    const wxString& begin_address, 
    const wxString& end_address) const;
  bool DoCommand(const wxString& command);
  void DoCommandLine();
  bool DoCommandRange(const wxString& command) const;
  void GotoBrace();
  void InsertMode(
    const wxUniChar c = 'i', 
    int repeat = 1,
    bool overtype = false);
  void Move(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& destination) const;
  bool SetSelection(
    const wxString& begin_address, 
    const wxString& end_address) const;
  void Substitute(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& pattern,
    const wxString& replacement) const;
  void ToggleCase() const;
  int ToLineNumber(const wxString& address) const;
  void Yank(int lines) const;
  void Yank(
    const wxString& begin_address, 
    const wxString& end_address) const;

  std::map<wxUniChar, int> m_Markers;
  bool m_InsertMode;
  bool m_SearchForward;
  int m_InsertRepeatCount;
  wxExSTC* m_STC;
  wxString m_Command;
  wxString m_InsertText;
  wxString m_LastCommand;
  wxString m_SearchText;
};

#endif // wxUSE_GUI
#endif
