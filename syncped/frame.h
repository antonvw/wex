////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/notebook.h>
#include <wex/process.h>
#include <wex/report/dirctrl.h>
#include <wex/report/listview.h>
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
  virtual wex::listview* activate(
    wex::listview_data::type_t type, const wex::lexer* lexer = nullptr) override;
  virtual bool exec_ex_command(wex::ex_command& command) override;
  virtual wex::process* get_process(const std::string& command) override;
  virtual wex::listview_file* get_project() override;
  virtual bool is_open(const wex::path& filename) override;
  virtual void on_command_item_dialog(
    wxWindowID dialogid, const wxCommandEvent& event) override;
  virtual wex::stc* open_file(
    const wex::path& filename,
    const wex::stc_data& stc_data = wex::stc_data()) override;
  virtual wex::stc* open_file(
    const wex::path& filename, 
    const wex::vcs_entry& vcs, 
    const wex::stc_data& stc_data = wex::stc_data()) override;
  virtual wex::stc* open_file(
    const wex::path& filename, 
    const std::string& text, 
    const wex::stc_data& stc_data = wex::stc_data()) override;
  virtual void print_ex(wex::ex* ex, const std::string& text) override;
  virtual wex::stc* restore_page(const std::string& key) override;
  virtual bool save_current_page(const std::string& key) override;
  virtual void statusbar_clicked(const std::string& pane) override;
  virtual void statusbar_clicked_right(const std::string& pane) override;
  virtual void sync_all() override;
  virtual void sync_close_all(wxWindowID id) override;
  
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
