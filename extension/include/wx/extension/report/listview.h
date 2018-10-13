////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wex::history_listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/listview.h>
#include <wx/extension/tool.h>

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
    static listview_type GetTypeTool(const tool& tool);
  protected:
    virtual void BuildPopupMenu(menu& menu) override;
    history_frame* GetFrame() {return m_Frame;};
  private:
    const long m_MenuFlags;
    history_frame* m_Frame;
  };
};
