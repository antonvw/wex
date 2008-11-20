/******************************************************************************\
* File:          stc.h
* Purpose:       Declaration of class 'ftSTC'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: stc.h 56 2008-11-14 19:14:37Z anton $
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _FTSTYLEDTEXTCTRL_H
#define _FTSTYLEDTEXTCTRL_H

class ftFrame;

#include <wx/extension/stc.h>

/// Adds a frame and drag/drop to exSTC.
class ftSTC : public exSTC
{
public:
  /// Menu types, they determine how the context menu will appear.
  /// These values extend the menu types used by exSTC.
  enum
  {
    STC_MENU_TOOL           = 0x0100, ///< for adding tool menu
    STC_MENU_REPORT_FIND    = 0x0200, ///< for adding find in files
    STC_MENU_REPORT_REPLACE = 0x0400, ///< for adding replace in files
    STC_MENU_COMPARE        = 0x1000, ///< for adding compare
  };

  /// Extra open flags.
  enum
  {
    STC_OPEN_IS_PROJECT = 0x0100,
  };

  /// Constructor. Does not open a file.
  ftSTC(wxWindow* parent,
    long type = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0,
    const wxString& name = wxSTCNameStr);

  /// Constructor, opens the file.
  ftSTC(wxWindow* parent,
    const wxString& filename,
    int line_number = 0, // goes to the line if > 0, if -1 goes to end of file
    const wxString& match = wxEmptyString, // and selects the text on that line
    long flags = 0,
    long type = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0,
    const wxString& name = wxSTCNameStr);

  /// Copy constructor.
  ftSTC(const ftSTC& stc);

  /// Calls base and sets recent file if base call succeeded.
  virtual bool Open(
    const wxString& filename,
    int line_number = 0, // goes to the line if > 0, if -1 goes to end of file
    const wxString& match = wxEmptyString,     // and selects the text on that line
    long flags = 0);

  /// Invokes base properties message and sets the frame title.
  virtual void PropertiesMessage();
protected:
  /// Builds the popup menu.
  virtual void BuildPopupMenu(exMenu& menu);

  void OnCommand(wxCommandEvent& command);

  DECLARE_EVENT_TABLE()
private:
  bool Initialize();
  ftFrame* m_Frame;
};

#endif
