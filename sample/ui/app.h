////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/app.h>
#include <wex/dir.h>
#include <wex/frame.h>
#include <wex/grid-statistics.h>
#include <wex/grid.h>
#include <wex/listview.h>
#include <wex/notebook.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wex/stc.h>

/// Derive your application from wex::app.
class app : public wex::app
{
private:
  bool OnInit() override;
};

class dir : public wex::dir
{
public:
  dir(
    const std::string& fullpath,
    const std::string& findfiles,
    wex::grid*         grid);

private:
  bool       on_file(const wex::path& file) override;
  wex::grid* m_grid;
};

class frame : public wex::frame
{
public:
  frame();

private:
  bool           allow_close(wxWindowID id, wxWindow* page) override;
  wex::listview* get_listview() override { return m_listview; };
  void
  on_command_item_dialog(wxWindowID id, const wxCommandEvent& event) override;

  void on_command(wxCommandEvent& event);

  wex::notebook*             m_notebook;
  wex::grid*                 m_grid;
  wex::listview*             m_listview;
  wex::process*              m_process;
  wex::shell*                m_shell;
  wex::grid_statistics<int>* m_statistics;
  wex::stc *                 m_stc, *m_stc_lexers;

  long m_flags_stc = 0;
};
