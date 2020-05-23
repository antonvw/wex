////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/report/listview.h>
#include "support.h"

class app;

class frame : public decorated_frame
{
public:
  explicit frame(app* app);
private:
  // All overrides.
  
  wex::report::listview* activate(
    wex::listview_data::type_t type, 
    const wex::lexer* lexer = nullptr) override;
  
  bool exec_ex_command(wex::ex_command& command) override;
  
  wex::process* get_process(const std::string& command) override;
  
  wex::report::file* get_project() override;
  
  bool is_open(const wex::path& filename) override;
  
  void on_command_item_dialog(
    wxWindowID dialogid, 
    const wxCommandEvent& event) override;
  
  wex::stc* open_file(
    const wex::path& filename,
    const wex::stc_data& stc_data = wex::stc_data()) override;
  
  wex::stc* open_file(
    const wex::path& filename, 
    const wex::vcs_entry& vcs, 
    const wex::stc_data& stc_data = wex::stc_data()) override;
  
  wex::stc* open_file(
    const wex::path& filename, 
    const std::string& text, 
    const wex::stc_data& stc_data = wex::stc_data()) override;
  
  void print_ex(wex::ex* ex, const std::string& text) override;
  
  void record(const std::string& command) override;
  
  wex::stc* restore_page(const std::string& key) override;
  
  bool save_current_page(const std::string& key) override;
  
  void statusbar_clicked(const std::string& pane) override;
  
  void sync_all() override;
  
  void sync_close_all(wxWindowID id) override;
  
  // All others.
  
  void on_command(wxCommandEvent& event);
  void on_update_ui(wxUpdateUIEvent& event);
  void update_listviews();

  bool m_maximized {false};

  int m_split_id {1};

  std::string m_saved_page;

  DECLARE_EVENT_TABLE()
};
