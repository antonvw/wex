////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wex::del::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/common/tool.h>
#include <wex/ui/listview.h>

namespace wex::del
{
class frame;
struct menu_env;

/// Adds a del::frame to listview.
/// It also adds a tool menu if appropriate.
class listview : public wex::listview
{
public:
  /// Default constructor.
  listview(const data::listview& data = data::listview());

  /// Destroys the window safely.
  bool Destroy() override;

  /// Returns list type from tool id.
  static data::listview::type_t type_tool(const tool& tool);

protected:
  void  build_popup_menu(menu& menu) override;
  auto* get_frame() { return m_frame; }

private:
  void build_popup_menu_multiple(const menu_env* env, menu& menu);
  void build_popup_menu_single(const menu_env* env, menu& menu);
  void on_compare();
  void on_tool(const wxCommandEvent& event);

  const data::listview::menu_t m_menu_flags;
  class frame*                 m_frame;
};
}; // namespace wex::del
