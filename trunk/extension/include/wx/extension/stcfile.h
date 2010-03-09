/******************************************************************************\
* File:          stcfile.h
* Purpose:       Declaration of class wxExSTC
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
class wxExVi;

#if wxUSE_GUI
/// Offers a styled text ctrl with find/replace, folding, printing, popup menu,
/// config support and syntax colouring. Also adds synchronizing to the window,
/// and if the file is a logfile and
/// the caret is at the end, it stays at the end after syncing.
class wxExSTC : public wxExStyledTextCtrl, public wxExFile
{
public:
  /// Menu and tooltip flags (0 is used for no menu).
  enum wxExSTCMenuFlags
  {
    STC_MENU_SIMPLE    = 0x0002, ///< for adding copy/paste etc. menu
    STC_MENU_FIND      = 0x0004, ///< for adding find menu
    STC_MENU_REPLACE   = 0x0008, ///< for adding replace menu
    STC_MENU_INSERT    = 0x0010, ///< for adding sequence menu
    STC_MENU_OPEN_LINK = 0x0020, ///< for adding link open menu

    STC_MENU_DEFAULT   = 0xFFFF, ///< all
  };

  /// Open flags (0 is used as default).
  enum wxExSTCOpenFlags
  {
    STC_OPEN_HEX         = 0x0001, ///< open in hex mode
    STC_OPEN_READ_ONLY   = 0x0002, ///< open as readonly, this mode overrides real mode from disk
    STC_OPEN_FROM_OTHER  = 0x0008, ///< opened from within another file (e.g. a link)
  };

  /// Config dialog flags (0 gives
  /// a modal dialog with all options).
  enum wxExSTCConfigFlags
  {
    STC_CONFIG_MODELESS   = 0x0001, ///< use as modeless dialog
    STC_CONFIG_WITH_APPLY = 0x0002, ///< add the apply button
    STC_CONFIG_SIMPLE     = 0x0004, ///< only 'simple' options on dialog
  };

  /// Constructor. Does not open a file, but sets text to specified value,
  /// NULL's are allowed.
  /// This default value is overwritten by Open.
  wxExSTC(wxWindow* parent,
    const wxString& value = wxEmptyString,
    long open_flags = 0,
    const wxString& title = wxEmptyString,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0,
    const wxString& name = wxSTCNameStr);

  /// Constructor, opens the file.
  /// See also Open.
  wxExSTC(wxWindow* parent,
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long open_flags = 0,
    long menu_flags = STC_MENU_DEFAULT,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = 0,
    const wxString& name = wxSTCNameStr);

  /// Destructor.
 ~wxExSTC();

  /// Copy constructor.
  wxExSTC(const wxExSTC& stc);

  /// Colourises the document.
  void Colourise();

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

  /// Invokes base and clears document.
  void FileNew(const wxExFileName& filename = wxExFileName());

  /// Shows a menu with current line type checked, and allows you to change it.
  void FileTypeMenu();

  virtual bool GetContentsChanged() const {return GetModify();};

  virtual void ResetContentsChanged();

  /// Gets current flags (used by Open).
  long GetFlags() const {return m_Flags;};

  /// Gets the menu flags.
  long GetMenuFlags() const {return m_MenuFlags;};

  /// Asks for a lexer for this document, choosing from a dialog of
  /// all available lexers. Then colours the document.
  void LexerDialog(const wxString& caption = _("Enter Lexer"));

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
  virtual void PropertiesMessage() const;

  /// If set, then the popup menu will show a file save item
  /// if the document is modified.
  /// Default it is off.
  void SetFileSaveInMenu(bool val = true) {
    m_FileSaveInMenu = val;}

  /// Set the (scintilla) lexer for this document using the current filename by
  /// default, or the explicit lexer if specified.
  /// Then colourises the document.
  void SetLexer(const wxString& lexer = wxEmptyString, bool forced = false);

  /// Sets the text.
  void SetText(const wxString& value);

#if wxUSE_STATUSBAR
  /// Updates the specified statusbar pane with current values.
  void UpdateStatusBar(const wxString& pane) const;
#endif
protected:
  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);
  virtual void DoFileLoad(bool synced = false);
  virtual void DoFileSave(bool save_as = false);

  void OnChar(wxKeyEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnKeyDown(wxKeyEvent& event);
  void OnKeyUp(wxKeyEvent& event);
  void OnMouse(wxMouseEvent& event);
  void OnStyledText(wxStyledTextEvent& event);
private:
  void AddTextHexMode(wxFileOffset start, const wxCharBuffer& buffer);
  void AddBasePathToPathList();
  bool CheckAutoComp(const wxUniChar c);
  bool CheckBrace(int pos);
  bool CheckBraceHex(int pos);
  // Clears the component: all text is cleared and all styles are reset.
  // Invoked by Open and FileNew.
  // (Clear is used by scintilla to clear the selection).
  void ClearDocument();
  void EOLModeUpdate(int eol_mode);
  bool FileReadOnlyAttributeChanged(); // sets changed read-only attribute
  void GuessType();
  void Initialize();
  bool LinkOpen(
    const wxString& link,
    wxString& filename, // name of found file
    int line_number = 0, 
    bool link_open = true);
  void ReadFromFile(bool get_only_new_data);
  void SetGlobalStyles();

  // All objects share the following:
  static wxExConfigDialog* m_ConfigDialog;

  bool m_FileSaveInMenu;
  bool m_viMode;
  long m_Flags; // open flags
  long m_MenuFlags;

  wxExVi* m_vi;
  wxFileOffset m_PreviousLength;
  wxPathList m_PathList;

  DECLARE_EVENT_TABLE()
};

#endif // wxUSE_GUI
#endif
