////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of sample classes for wxExtension
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/app.h>
#include <wx/extension/dir.h>
#include <wx/extension/grid.h>
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/notebook.h>
#include <wx/extension/process.h>
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
  virtual bool OnInit() override;
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
  virtual void OnFile(const wxString& file) override;
  wxExGrid* m_Grid;
};
#endif

/// Use wxExManagedFrame.
class wxExSampleFrame: public wxExManagedFrame
{
public:
  /// Constructor.
  wxExSampleFrame();
  virtual wxExSTC* GetSTC() override {
    return m_STC;};
  virtual void OnCommandConfigDialog(
    wxWindowID id, 
    const wxCommandEvent& event) override;
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  void ShowConfigItems();
  
#if wxUSE_GRID
  wxExGrid* m_Grid;
#endif
  wxExListView* m_ListView;
  wxExNotebook* m_Notebook;
  wxExProcess* m_Process;
  wxExSTC* m_STC;
  wxExSTC* m_STCLexers;
  wxExShell* m_Shell;

  long m_FlagsSTC;
  wxExStatistics <int> m_Statistics;

  DECLARE_NO_COPY_CLASS(wxExSampleFrame)
};
