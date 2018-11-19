////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wex::history_listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/listview.h>
#include <wex/tool.h>

namespace wex
{
  class history_frame;

  /// Adds a history_frame to listview.
  /// It also adds a tool menu if appropriate.
  class history_listview : public listview
  {
  public:
    /// Default constructor.
    history_listview(const listview_data& data = listview_data());
      
    /// Destroys the window safely.
    virtual bool Destroy() override;

    /// Returns list type from tool id.
    static listview_data::type_t type_tool(const tool& tool);
  protected:
    virtual void build_popup_menu(menu& menu) override;
    history_frame* frame() {return m_Frame;};
  private:
    const listview_data::menu_t m_MenuFlags;
    history_frame* m_Frame;
  };
};
