////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of sample classes for wxExtension
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/app.h>
#include <wx/extension/dir.h>
#include <wx/extension/grid.h>
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/notebook.h>
#include <wx/extension/shell.h>
#include <wx/extension/statistics.h>
#include <wx/extension/stc.h>

/// Derive your application from wxExApp.
class wxExSampleApp: public wxExApp
{
public:
  /// Constructor.
  wxExSampleApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
  DECLARE_NO_COPY_CLASS(wxExSampleApp)
};

#if wxUSE_GRID
/// Use wxExDir.
class wxExSampleDir: public wxExDir
{
public:
  /// Constructor.
  wxExSampleDir(
    const wxString& fullpath, 
    const wxString& findfiles, 
    wxExGrid* grid);
private:
  /// Override the OnFile.
  virtual void OnFile(const wxString& file);
  wxExGrid* m_Grid;
};
#endif

/// Use wxExManagedFrame.
class wxExSampleFrame: public wxExManagedFrame
{
public:
  /// Constructor.
  wxExSampleFrame();
protected:
  virtual void OnCommandConfigDialog(
    wxWindowID id, 
    int commandid = wxID_APPLY);
  void OnCommand(wxCommandEvent& event);
private:
  virtual wxExListView* GetListView();
  void ShowConfigItems();
  
#if wxUSE_GRID
  wxExGrid* m_Grid;
#endif
  wxExListView* m_ListView;
  wxExNotebook* m_Notebook;
  wxExSTC* m_STC;
  wxExSTC* m_STCLexers;
  wxExSTCShell* m_STCShell;

  long m_FlagsSTC;
  wxExStatistics <long> m_Statistics;

  DECLARE_NO_COPY_CLASS(wxExSampleFrame)
  DECLARE_EVENT_TABLE()
};
