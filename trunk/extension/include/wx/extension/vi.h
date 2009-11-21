////////////////////////////////////////////////////////////////////////////////
// Name:      vi.h
// Purpose:   Declaration of class wxExVi
// Author:    Anton van Wezenbeek
// Created:   2009-11-21
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/textdlg.h> 
#include <wx/extension/stc.h>
#include <wx/extension/frd.h>

#if wxUSE_GUI

class wxExVi
{
public:
  wxExVi(wxExSTC* stc);
  bool GetInsertMode() const {return m_InsertMode;};
  void OnKey(wxKeyEvent& event);
private:
  void Run(const wxString& command);
  wxExSTC* m_STC;
  wxString m_viCommand;
  bool m_InsertMode;
};

#endif // wxUSE_GUI
