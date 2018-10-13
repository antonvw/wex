////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/notebook.h>
#include <wx/extension/process.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/listview.h>
#include "support.h"

class app;
class editors;

class frame : public decorated_frame
{
public:
  explicit frame(app* app);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wex::listview* Activate(
    wex::listview_type type, const wex::lexer* lexer = nullptr) override;
  virtual bool ExecExCommand(wex::ex_command& command) override;
  virtual wex::listview_file* GetProject() override;
  virtual bool IsOpen(const wex::path& filename) override;
  virtual void OnCommandItemDialog(
    wxWindowID dialogid, const wxCommandEvent& event) override;
  virtual wex::stc* OpenFile(
    const wex::path& filename,
    const wex::stc_data& stc_data = wex::stc_data()) override;
  virtual wex::stc* OpenFile(
    const wex::path& filename, 
    const wex::vcs_entry& vcs, 
    const wex::stc_data& stc_data = wex::stc_data()) override;
  virtual wex::stc* OpenFile(
    const wex::path& filename, 
    const std::string& text, 
    const wex::stc_data& stc_data = wex::stc_data()) override;
  virtual void PrintEx(wex::ex* ex, const std::string& text) override;
  virtual wex::process* Process(const std::string& command) override;
  virtual wex::stc* RestorePage(const std::string& key) override;
  virtual bool SaveCurrentPage(const std::string& key) override;
  virtual void StatusBarClicked(const std::string& pane) override;
  virtual void StatusBarClickedRight(const std::string& pane) override;
  virtual void SyncAll() override;
  virtual void SyncCloseAll(wxWindowID id) override;
  
  void AddPaneHistory();
  void AddPaneProcess();
  void AddPaneProjects();

  const long m_PaneFlag = 
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_CLOSE_ON_ALL_TABS |
    wxAUI_NB_CLOSE_BUTTON |
    wxAUI_NB_WINDOWLIST_BUTTON |
    wxAUI_NB_SCROLL_BUTTONS;
  const wxString m_ProjectWildcard = 
    _("Project Files") + " (*.prj)|*.prj";

  bool m_Maximized {false};
  int m_NewProjectNo {1}, m_SplitId {1};

  editors *m_Editors {nullptr};

  wxCheckBox *m_CheckBoxDirCtrl {nullptr}, *m_CheckBoxHistory {nullptr};
  wex::dirctrl* m_DirCtrl {nullptr};
  wex::history_listview* m_History {nullptr};
  wex::notebook *m_Lists {nullptr}, *m_Projects {nullptr};
  wex::process* m_Process {nullptr};
  wex::stc* m_asciiTable {nullptr};
  std::string m_SavedPage;

  DECLARE_EVENT_TABLE()
};
