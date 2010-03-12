/******************************************************************************\
* File:          frame.h
* Purpose:       Declaration of wxexFrame class and support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXFRAME_H
#define _EXFRAME_H

#include <map>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aui/auibar.h> // for wxAuiToolBar
#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/fdrepdlg.h> // for wxFindDialogDialog and Event
#include <wx/extension/defs.h> // for ID_EDIT_STATUS_BAR
#include <wx/extension/file.h> // for wxExFileName

// Only if we have a gui.
#if wxUSE_GUI

class wxExGrid;
class wxExListView;
class wxExStatusBar;
class wxExSTCFile;
class wxExSTC;
class wxExToolBar;

#if wxUSE_STATUSBAR
/// This class defines our statusbar panes, to be used by wxExFrame::SetupStatusBar.
/// It just adds some members to the base class, and keeps a static total.
class wxExPane : public wxStatusBarPane
{
  friend class wxExStatusBar;
public:
  /// Default constructor.
  wxExPane(
    /// If you do no provide helptext, it is derived from the name, by using
    /// text after the first 'e' character (so after 'Pane') if name is
    /// not 'PaneText'.
    const wxString& name = wxEmptyString,
    /// Width of the pane.
    int width = 50,
    /// The helptext shown as a tooltip.
    const wxString& helptext = wxEmptyString,
    /// The style.
    int style = wxSB_NORMAL)
    : wxStatusBarPane(style, width)
    , m_Helptext(
        helptext.empty() && name != "PaneText" ? 
          name.AfterFirst('e'): helptext)
    , m_Name(name)
    , m_No(m_Total)
    {m_Total++;};
private:
  wxString m_Helptext;
  wxString m_Name;
  int m_No;
  static int m_Total;
};
#endif // wxUSE_STATUSBAR

/// Offers a frame with easy statusbar methods, 
/// find/replace, and a toolbar if you call CreateToolBar.
/// Allows for file dropping as well.
class wxExFrame : public wxFrame
{
public:
  /// Flags for StatusText.
  enum wxExStatusFlags
  {
    STAT_DEFAULT  = 0x0000, ///< shows 'modified' and file 'fullname'
    STAT_SYNC     = 0x0001, ///< shows 'synchronized' instead of 'modified'
    STAT_FULLPATH = 0x0002, ///< shows file 'fullpath' instead of 'fullname'
  };

  /// Constructor, the frame position and size is taken from the config.
  wxExFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE,
    const wxString& name = wxFrameNameStr);

  /// Using specified position and size.
  wxExFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size,
    long style = wxDEFAULT_FRAME_STYLE,
    const wxString& name = wxFrameNameStr);

  /// Destructor, deletes the statusbar.
 ~wxExFrame();

  /// Returns a grid, default returns the focused grid.
  virtual wxExGrid* GetGrid() {return GetFocusedGrid();};

  /// Returns a listview, default returns the focused listview.
  virtual wxExListView* GetListView() {return GetFocusedListView();};

  /// Returns an STC, default returns the focused STC.
  virtual wxExSTCFile* GetSTC() {return GetFocusedSTC();};

  /// If the window that has focus is a Grid, then returns that, 
  /// otherwise returns NULL.
  wxExGrid* GetFocusedGrid();

  /// If the window that has focus is a ListView, 
  /// then returns that, otherwise returns NULL.
  wxExListView* GetFocusedListView();

  /// If the window that has focus is an STC, then returns that, 
  /// otherwise returns NULL.
  wxExSTCFile* GetFocusedSTC();

  /// Called when a config dialog command event is triggered.
  /// Default it fires when the apply button was pressed.
  virtual void OnCommandConfigDialog(
    wxWindowID WXUNUSED(dialogid),
    int WXUNUSED(commandid) = wxID_APPLY) {};

  /// Default opens the file using the GetFocusedSTC.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

#if wxUSE_STATUSBAR
  /// Do something when statusbar is clicked.
  virtual void StatusBarClicked(
    const wxString& WXUNUSED(pane)) {};

  /// When double clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarDoubleClicked(
    const wxString& pane);

  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static void StatusText(
    const wxString& text, 
    const wxString& pane = "PaneText");

  /// Shows filename info on the statusbar.
  // Using type wxExStatusFlags instead of long gives compiler errors at
  // invoking.
  static void StatusText(
    const wxExFileName& filename, 
    long flags = STAT_DEFAULT);
#endif // wxUSE_STATUSBAR

protected:
  /// Invokes GetSearchText on one of the controls.
  void GetSearchText();

  /// Writes the current frame size and position to the config.
  void OnClose(wxCloseEvent& event);
  /// Handles command event.
  void OnCommand(wxCommandEvent& command);

#if wxUSE_STATUSBAR
  // Interface from wxFrame.
  virtual wxStatusBar* OnCreateStatusBar(int number,
    long style,
    wxWindowID id,
    const wxString& name);
#endif

#if wxUSE_TOOLBAR
  // Interface from wxFrame.
  virtual wxToolBar* OnCreateToolBar(
    long style,
    wxWindowID id,
    const wxString& name);
#endif

  /// If there is a STC, calls find.
  void OnFindDialog(wxFindDialogEvent& event);
  
  /// If there is a focused STC, updates the status bar.
  void OnUpdateUI(wxUpdateUIEvent& event);
  
#if wxUSE_STATUSBAR
  /// Sets up the status bar if you want to use StatusText.
  void SetupStatusBar(
    const std::vector<wxExPane>& panes,
    long style = wxST_SIZEGRIP,
    wxWindowID id = ID_EDIT_STATUS_BAR,
    const wxString& name = "statusBar");
#endif
protected:
#if wxUSE_TOOLBAR
  wxExToolBar* m_ToolBar;
#endif

private:
  void FindIn(wxFindDialogEvent& event, wxExGrid* grid);
  void FindIn(wxFindDialogEvent& event, wxExListView* lv);
  void FindIn(wxFindDialogEvent& event, wxExSTC* stc);
  void Initialize();

#if wxUSE_STATUSBAR
  static wxExStatusBar* m_StatusBar;
#endif

  wxFindReplaceDialog* m_FindReplaceDialog;

  wxExGrid* m_FocusGrid;
  wxExListView* m_FocusListView;
  wxExSTC* m_FocusSTC;

  const bool m_KeepPosAndSize;

  DECLARE_EVENT_TABLE()
};

#if wxUSE_AUI
/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes.
class wxExManagedFrame : public wxExFrame
{
public:
  /// Constructor, the frame position and size is taken from the config.
  wxExManagedFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE,
    const wxString& name = wxFrameNameStr);

  /// Destructor.
 ~wxExManagedFrame();

  // Interface for notebooks.
  /// Returns true if the page can be closed.
  virtual bool AllowClose(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {return true;}

  /// Called if the notebook changed page.
  virtual void OnNotebook(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {;};

  /// Called after all pages from the notebooks are deleted.
  virtual void SyncCloseAll(wxWindowID WXUNUSED(id)) {;};

  /// Gets the manager.
  wxAuiManager& GetManager() {return m_Manager;};

  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  void TogglePane(const wxString& pane);
private:
  wxAuiManager m_Manager;
};
#endif

#if wxUSE_STATUSBAR
/// Offers a status bar with popup menu in relation to wxExFrame.
class wxExStatusBar : public wxStatusBar
{
public:
  /// Constructor.
  wxExStatusBar(wxExFrame* parent,
    wxWindowID id = wxID_ANY,
    long style = wxST_SIZEGRIP,
    const wxString& name = wxStatusBarNameStr);

  /// Sets the panes.
  void SetPanes(const std::vector<wxExPane>& panes);

  /// Sets text on specified pane.
  void SetStatusText(
    const wxString& text, 
    const wxString& pane = "PaneText");
protected:
  void OnMouse(wxMouseEvent& event);
private:
  /// Returns the status bar pane.
  /// If pane could not be found, returns empty pane.
  const wxExPane GetPane(int pane) const;

  wxExFrame* m_Frame;
  std::map<wxString, wxExPane> m_Panes;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
/// Offers a toolbar together with stock art.
class wxExToolBar : public wxToolBar
{
public:
  /// Constructor.
  wxExToolBar(wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxTB_HORIZONTAL,
    const wxString& name = wxToolBarNameStr);

  /// Adds automatic naming (for stock menu id's) and art id for toolbar normal items.
  wxToolBarToolBase* AddTool(int toolId);
};
#endif // wxUSE_TOOLBAR

class ComboBox;

#if wxUSE_AUI
/// Offers a find toolbar, containing a find combobox, up and down arrows
/// and checkboxes.
/// The find combobox allows you to find in an wxExSTCFile
/// component on the specified wxExFrame.
class wxExFindToolBar : public wxAuiToolBar
{
public:
  /// Constructor.
  wxExFindToolBar(
    wxWindow* parent, 
    wxExFrame* frame, 
    wxWindowID id = wxID_ANY);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  void Initialize();

  wxCheckBox* m_RegularExpression;
  wxCheckBox* m_MatchCase;
  wxCheckBox* m_MatchWholeWord;
  wxExFrame* m_Frame;
  ComboBox* m_ComboBox;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_AUI
#endif // wxUSE_GUI
#endif
