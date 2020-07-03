////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/app.h>
#include <wex/otl.h>
#include <wex/statistics.h>
#include <wex/report/frame.h>

namespace wex
{
  class grid;
  class shell;
  class stc;
};

class app: public wex::app
{
private:
  bool OnInit() override;
};

class frame: public wex::report::frame
{
public:
  frame();
private:
  void on_command_item_dialog(
    wxWindowID dialogid, 
    const wxCommandEvent& event) override;

  wex::stc* open_file(
    const wex::path& filename, 
    const wex::data::stc& data = wex::data::stc()) override;

  void statusbar_clicked(const std::string& pane) override;

  void run_query(const std::string& query, bool empty_results = false);

  wex::grid* m_results;
  wex::otl m_otl;
  wex::shell* m_shell;
  wex::statistics <int> m_statistics;
  wex::stc* m_query;
  
  bool 
    m_running {false}, 
    m_stopped {false};
};
