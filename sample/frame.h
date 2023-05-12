////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wex sample class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/wex.h>

/// Derive your application from wex::del::frame.
class frame : public wex::del::frame
{
public:
  frame();

  void update(app* a);

private:
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
  void open_file_same_page(const wex::path& p) final;

  wex::notebook*             m_notebook{nullptr};
  wex::stc*                  m_stc{nullptr};
  wex::grid*                 m_grid{nullptr};
  wex::listview*             m_listview{nullptr};
  wex::del::file*            m_project{nullptr};
  wex::process*              m_process{nullptr};
  wex::shell*                m_shell{nullptr};
  wex::grid_statistics<int>* m_statistics{nullptr};

  long m_flags_stc{0};
};
