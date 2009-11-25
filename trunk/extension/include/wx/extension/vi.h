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

  /// Handle key events.
  /// Returns true if event is allowed to be skipped.
  bool OnKey(wxKeyEvent& event);
private:
  void Delete(
    const wxString& begin_address, 
    const wxString& end_address);
  void DoCommand(const wxString& command);
  void InsertMode();
  void LineEditor(const wxString& command);
  void Move(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& destination);
  bool SetSelection(
    const wxString& begin_address, 
    const wxString& end_address);
  void Substitute(
    const wxString& begin_address, 
    const wxString& end_address, 
    const wxString& pattern,
    const wxString& replacement);
  int ToLineNumber(const wxString& address) const;
  void Yank(
    const wxString& begin_address, 
    const wxString& end_address);

  bool m_InsertMode;
  wxExSTC* m_STC;
  wxString m_Command;
  wxString m_InsertText;
  wxString m_LastCommand;
  wxString m_SearchText;
};

#endif // wxUSE_GUI
#endif
