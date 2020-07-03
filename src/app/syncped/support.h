////////////////////////////////////////////////////////////////////////////////
// Name:      support.h
// Purpose:   Declaration of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma

#include <wex/notebook.h>
#include <wex/process.h>
#include <wex/report/dirctrl.h>
#include <wex/report/frame.h>
#include <wex/report/listview.h>

class app;

class editors : public wex::notebook
{
public:
  editors(const wex::data::window& data);

  bool is_split() const { return m_split; };
  void reset() { m_split = false; };

private:
  bool m_split{false};
};

class decorated_frame : public wex::report::frame
{
public:
  decorated_frame(app* app);

protected:
  void add_pane_history();

  editors*               m_editors{nullptr};
  wex::report::dirctrl*  m_dirctrl{nullptr};
  wex::report::listview* m_history{nullptr};

  app* m_app;

  wex::notebook *m_lists{nullptr}, *m_projects{nullptr};

  wex::process* m_process{nullptr};

private:
  bool allow_close(wxWindowID id, wxWindow* page) override;
  void on_notebook(wxWindowID id, wxWindow* page) override;

  const std::string m_project_wildcard{_("Project Files") + " (*.prj)|*.prj"};

  int m_project_id{1};
};
