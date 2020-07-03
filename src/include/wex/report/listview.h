////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wex::report::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/listview.h>
#include <wex/tool.h>

namespace wex::report
{
  class frame;

  /// Adds a report::frame to listview.
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
    void   build_popup_menu(menu& menu) override;
    frame* get_frame() { return m_frame; };

  private:
    const data::listview::menu_t m_menu_flags;
    class frame*                 m_frame;
  };
}; // namespace wex::report
