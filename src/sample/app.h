////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019
////////////////////////////////////////////////////////////////////////////////

#include <wex/app.h>
#include <wex/dir.h>
#include <wex/grid.h>
#include <wex/listview.h>
#include <wex/managedframe.h>
#include <wex/notebook.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wex/statistics.h>
#include <wex/stc.h>

/// Derive your application from wex::app.
class app: public wex::app
{
public:
  app() {}
private:
  bool OnInit() override;
};

class dir: public wex::dir
{
public:
  dir(
    const std::string& fullpath, 
    const std::string& findfiles, 
    wex::grid* grid);
private:
  bool on_file(const wex::path& file) override;
  wex::grid* m_grid;
};

class frame: public wex::managed_frame
{
public:
  frame();
  wex::listview* get_listview() override {return m_listview;};
  void on_command_item_dialog(
    wxWindowID id, 
    const wxCommandEvent& event) override;
protected:
  void on_command(wxCommandEvent& event);
private:
  wex::notebook* m_notebook;
  wex::grid* m_grid;
  wex::listview* m_listview;
  wex::process* m_process;
  wex::stc* m_stc;
  wex::stc* m_stc_lexers;
  wex::shell* m_shell;

  long m_flags_stc = 0;
  wex::statistics <int> m_statistics;
};
