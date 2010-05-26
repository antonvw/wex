/******************************************************************************\
* File:          stcfile.h
* Purpose:       Declaration of class wxExSTCFile
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXSTCFILE_H
#define _EXSTCFILE_H

#include <wx/extension/stc.h>
#include <wx/extension/file.h> // for wxExFile
#include <wx/extension/menu.h> // for wxExMenu

class wxExConfigDialog;

#if wxUSE_GUI
/// Adds file support, config support and synchronizing to the window,
/// and if the file is a logfile and
/// the caret is at the end, it stays at the end after syncing.
class wxExSTCFile : public wxExSTC, public wxExFile
{
public:
  enum wxExSTCMenuFlags
  {
    STC_MENU_OPEN_LINK = 0x0020, ///< for adding link open menu
  };

  /// Config dialog flags (0 gives
  /// a modal dialog with all options).
  enum wxExSTCConfigFlags
  {
    STC_CONFIG_MODELESS   = 0x0001, ///< use as modeless dialog
    STC_CONFIG_WITH_APPLY = 0x0002, ///< add the apply button
    STC_CONFIG_SIMPLE     = 0x0004, ///< only 'simple' options on dialog
  };

  enum wxExSTCFileWindowFlags
  {
    STC_WIN_FROM_OTHER  = 0x0020, ///< opened from within another file (e.g. a link)
  };

  /// Constructor. Does not open a file, but sets text to specified value,
  /// NULL's are allowed.
  /// This default value is overwritten by Open.
  wxExSTCFile(wxWindow* parent,
    const wxString& value = wxEmptyString,
    long win_flags = 0,
    const wxString& title = wxEmptyString,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0);

  /// Constructor, opens the file.
  /// See also Open.
  wxExSTCFile(wxWindow* parent,
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long win_flags = 0,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0);

  /// Copy constructor.
  wxExSTCFile(const wxExSTCFile& stc);

  /// Adds a header.
  void AddHeader();

  /// Shows a dialog with options, returns dialog return code.
  /// If used modeless, it uses the dialog id as specified,
  /// so you can use that id in wxExFrame::OnCommandConfigDialog.
  static int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Editor Options"),
    long flags = 0,
    wxWindowID id = wxID_ANY);

  /// Sets the configurable parameters to values currently in config.
  void ConfigGet();

  virtual bool GetContentsChanged() const {return GetModify();};

  virtual void ResetContentsChanged();

  /// Opens the file, reads the content into the window, then closes the file
  /// and sets the lexer.
  /// If you specify a line number, goes to the line if > 0, if -1 goes to end of file.
  /// If you specify a match selects the text on that line.
  virtual bool Open(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

  /// Shows properties on the statusbar.
  virtual void PropertiesMessage();
protected:
  virtual void BuildPopupMenu(wxExMenu& menu);
  virtual void DoFileLoad(bool synced = false);
  virtual void DoFileNew();
  virtual void DoFileSave(bool save_as = false);

  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnMouse(wxMouseEvent& event);
private:
  void AddBasePathToPathList();
  bool FileReadOnlyAttributeChanged(); // sets changed read-only attribute
  bool LinkOpen(
    const wxString& link,
    wxString& filename, // name of found file
    int line_number = 0, 
    bool link_open = true);
  void ReadFromFile(bool get_only_new_data);

  // All objects share the following:
  static wxExConfigDialog* m_ConfigDialog;

  wxFileOffset m_PreviousLength;
  wxPathList m_PathList;

  DECLARE_EVENT_TABLE()
};

#endif // wxUSE_GUI
#endif
