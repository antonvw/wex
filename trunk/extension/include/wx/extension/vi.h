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

  /// Access to the insert mode.
  bool GetInsertMode() const {return m_InsertMode;};

  /// Handle key events.
  void OnKey(wxKeyEvent& event);

  /// Resets insert mode.
  void ResetInsertMode() {m_InsertMode = false;};
private:
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

  wxExSTC* m_STC;
  wxString m_Command;
  bool m_InsertMode;
};

#endif // wxUSE_GUI
#endif
