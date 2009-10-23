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
#include <wx/aui/auibar.h>
#include <wx/aui/auibook.h> // for wxAuiManager
#include <wx/datetime.h>
#include <wx/fdrepdlg.h> // for wxFindDialogDialog and Event
#include <wx/extension/defs.h> // for ID_EDIT_STATUS_BAR
#include <wx/extension/file.h> // for wxExFileName

// Only if we have a gui.
#if wxUSE_GUI

class wxExGrid;
class wxExListView;
class wxExStatusBar;
class wxExSTC;
class wxExToolBar;

#if wxUSE_STATUSBAR
/// This class defines our statusbar panes, to be used by wxExFrame::SetupStatusBar.
/// It just adds some members to the base class, and keeps a static total.
class wxExPane : public wxStatusBarPane
{
  friend class wxExFrame;
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
    , m_Helptext(helptext.empty() && name != "PaneText" ? name.AfterFirst('e'): helptext)
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

/// Flags for wxExStatusText.
enum wxExStatusFlags
{
  STAT_DEFAULT  = 0x0000, ///< shows 'modified' and file 'fullname'
  STAT_SYNC     = 0x0001, ///< shows 'synchronized' instead of 'modified'
  STAT_FULLPATH = 0x0002, ///< shows file 'fullpath' instead of 'fullname'
};

/// Offers a frame with easy statusbar methods, 
/// find/replace, and a toolbar if you call CreateToolBar.
class wxExFrame : public wxFrame
{
  friend class wxExStatusBar;
public:
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

  /// Called when a config dialog apply button was pressed.
  virtual void ConfigDialogApplied(wxWindowID WXUNUSED(dialogid)) {};

  /// Returns a grid, default returns the focused grid.
  virtual wxExGrid* GetGrid() {return GetFocusedGrid();};

  /// Returns a listview, default returns the focused listview.
  virtual wxExListView* GetListView() {return GetFocusedListView();};

  /// Returns an STC, default returns the focused STC.
  virtual wxExSTC* GetSTC() {return GetFocusedSTC();};

  /// If the window that has focus is a Grid, then returns that, 
  /// otherwise returns NULL.
  wxExGrid* GetFocusedGrid();

  /// If the window that has focus is a ListView, 
  /// then returns that, otherwise returns NULL.
  wxExListView* GetFocusedListView();

  /// If the window that has focus is an STC, then returns that, 
  /// otherwise returns NULL.
  wxExSTC* GetFocusedSTC();

  /// Default opens the file using the GetFocusedSTC.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

#if wxUSE_STATUSBAR
  /// Returns the status bar pane.
  /// If pane could not be found, returns empty pane.
  const wxExPane GetPane(int pane) const;

  /// Returns the field number of status bar pane.
  /// If pane could not be fonud, returns -1.
  static int GetPaneField(const wxString& pane);

  /// Do something when statusbar is clicked.
  virtual void StatusBarClicked(
    int WXUNUSED(field), 
    const wxPoint& WXUNUSED(point)) {};

  /// When double clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarDoubleClicked(int field, const wxPoint& point);

  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static void StatusText(const wxString& text, const wxString& pane = "PaneText");

  /// Shows filename info on the statusbar.
  static void StatusText(const wxExFileName& filename, long flags = STAT_DEFAULT);
#endif // wxUSE_STATUSBAR

protected:
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
  void GetSearchText();
  void Initialize();

#if wxUSE_STATUSBAR
  static wxExStatusBar* m_StatusBar;
  static std::map<wxString, wxExPane> m_Panes;
#endif

  wxFindReplaceDialog* m_FindReplaceDialog;

  const bool m_KeepPosAndSize;

  DECLARE_EVENT_TABLE()
};

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
protected:
  void OnMouse(wxMouseEvent& event);
private:
  wxExFrame* m_Frame;

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

  /// Adds automatic naming (for stock menu id's) and art id for toolbar check items.
  /// And for check tools as well.
  wxToolBarToolBase* AddCheckTool(int toolId);

  /// Adds automatic naming (for stock menu id's) and art id for toolbar normal items.
  wxToolBarToolBase* AddTool(int toolId);
};
#endif // wxUSE_TOOLBAR

class ComboBox;

/// Offers a find toolbar, containing a find combobox, up and down arrows
/// and checkboxes.
/// The find combobox allows you to find in an wxExSTC
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
private:
  wxCheckBox* m_RegularExpression;
  wxCheckBox* m_MatchCase;
  wxCheckBox* m_MatchWholeWord;
  wxExFrame* m_Frame;
  ComboBox* m_ComboBox;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
