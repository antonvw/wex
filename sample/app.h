////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>

/// Derive your application from wex::app.
class app : public wex::app
{
private:
  bool OnInit() final;
};

class dir : public wex::dir
{
public:
  dir(const wex::path& path, const std::string& findfiles, wex::grid* grid);

private:
  bool       on_file(const wex::path& file) final;
  wex::grid* m_grid;
};

/// Derive your application from wex::del::frame.
class frame : public wex::del::frame
{
public:
  frame();

private:
  void on_command(wxCommandEvent& event);

  wex::del::listview* activate(
    wex::data::listview::type_t type,
    const wex::lexer*           lexer = nullptr) final;
  bool           allow_close(wxWindowID id, wxWindow* page) final;
  wex::listview* get_listview() final;
  wex::stc*      get_stc() final;
  void on_command_item_dialog(wxWindowID id, const wxCommandEvent& event) final;
  wex::stc* open_file(
    const wex::path&      file,
    const wex::data::stc& data = wex::data::stc()) final;

  wex::notebook*             m_notebook;
  wex::stc *                 m_stc, *m_stc_lexers;
  wex::grid*                 m_grid;
  wex::listview*             m_listview;
  wex::process*              m_process;
  wex::shell*                m_shell;
  wex::grid_statistics<int>* m_statistics;

  long m_flags_stc = 0;
};
