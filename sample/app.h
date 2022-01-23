////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>

/// Derive your application from wex::app.
class app : public wex::app
{
public:
  auto& data() { return m_data; }
  auto& get_files() const { return m_files; }

private:
  bool OnInit() final;

  std::vector<wex::path> m_files;
  wex::data::stc         m_data;
};

class dir : public wex::dir
{
public:
  dir(const wex::path& path, const std::string& findfiles, wex::grid* grid);

private:
  bool       on_file(const wex::path& file) const final;
  wex::grid* m_grid;
};

/// Derive your application from wex::del::frame.
class frame : public wex::del::frame
{
public:
  frame();

  void update(app* a);

private:
  void add_data();
  void bind_all();
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

  wex::notebook*             m_notebook{nullptr};
  wex::stc *                 m_stc{nullptr}, *m_stc_lexers{nullptr};
  wex::grid*                 m_grid{nullptr};
  wex::listview*             m_listview{nullptr};
  wex::process*              m_process{nullptr};
  wex::shell*                m_shell{nullptr};
  wex::grid_statistics<int>* m_statistics{nullptr};

  long m_flags_stc = 0;
};
