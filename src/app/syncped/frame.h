////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "support.h"
#include <wex/report/listview.h>

class app;

class frame : public decorated_frame
{
public:
  explicit frame(app* app);

private:
  // All overrides.

  wex::report::listview* activate(
    wex::data::listview::type_t type,
    const wex::lexer*           lexer = nullptr) override;

  bool exec_ex_command(wex::ex_command& command) override;

  wex::process* get_process(const std::string& command) override;

  wex::report::file* get_project() override;

  bool is_open(const wex::path& filename) override;

  void on_command_item_dialog(wxWindowID dialogid, const wxCommandEvent& event)
    override;

  wex::stc* open_file(
    const wex::path&      filename,
    const wex::data::stc& data = wex::data::stc()) override;

  wex::stc* open_file(
    const wex::path&      filename,
    const wex::vcs_entry& vcs,
    const wex::data::stc& data = wex::data::stc()) override;

  wex::stc* open_file(
    const wex::path&      filename,
    const std::string&    text,
    const wex::data::stc& data = wex::data::stc()) override;

  bool output(const std::string& text) const override;

  void print_ex(wex::ex* ex, const std::string& text) override;

  void record(const std::string& command) override;

  wex::stc* restore_page(const std::string& key) override;

  bool save_current_page(const std::string& key) override;

  void statusbar_clicked(const std::string& pane) override;

  bool
  statustext(const std::string& text, const std::string& pane) const override;

  void sync_all() override;

  void sync_close_all(wxWindowID id) override;

  // All others.

  void on_command(wxCommandEvent& event);
  void on_update_ui(wxUpdateUIEvent& event);
  void provide_output(const std::string& text) const;
  void update_listviews();

  bool m_maximized{false};

  int m_split_id{1};

  std::string m_saved_page;

  DECLARE_EVENT_TABLE()
};
