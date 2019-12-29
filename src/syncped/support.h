////////////////////////////////////////////////////////////////////////////////
// Name:      support.h
// Purpose:   Declaration of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma

#include <wex/report/frame.h>
#include <wex/notebook.h>
#include <wex/process.h>

class app;

class editors : public wex::notebook
{
public:
  editors(const wex::window_data& data); 

  bool is_split() const {return m_split;};
  void reset() {m_split = false;};
private:
  bool m_split {false};
};

class decorated_frame : public wex::report::frame
{
public:
  decorated_frame(app* app);
protected:
  void add_pane_history();

  const long m_pane_flag = 
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_CLOSE_ON_ALL_TABS |
    wxAUI_NB_CLOSE_BUTTON |
    wxAUI_NB_WINDOWLIST_BUTTON |
    wxAUI_NB_SCROLL_BUTTONS;

  editors *m_editors {nullptr};
  wex::report::listview* m_history {nullptr};
  
  app* m_app;

  wex::notebook 
    *m_lists {nullptr}, 
    *m_projects {nullptr};

  wex::process* m_process {nullptr};
private:
  bool allow_close(wxWindowID id, wxWindow* page) override;
  void on_notebook(wxWindowID id, wxWindow* page) override;

  const std::string m_project_wildcard {_("Project Files") + " (*.prj)|*.prj"};

  int m_project_id {1};

  wex::stc* m_ascii_table {nullptr};
};
