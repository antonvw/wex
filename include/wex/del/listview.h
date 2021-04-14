////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wex::del::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/listview.h>
#include <wex/path-match.h>
#include <wex/queue-thread.h>
#include <wex/tool.h>

namespace wex::del
{
  class frame;

  /// Adds a del::frame to listview.
  /// It also adds a tool menu if appropriate.
  class listview
    : public wex::listview
#ifdef USE_QUEUE
    , public queue_thread<path_match>::event_handler
#endif
  {
  public:
    /// Default constructor.
    listview(const data::listview& data = data::listview());

    /// Processes match.
    void process_match(const path_match& input);

    /// Destroys the window safely.
    bool Destroy() override;

    /// Returns list type from tool id.
    static data::listview::type_t type_tool(const tool& tool);

  protected:
    void  build_popup_menu(menu& menu) override;
    auto* get_frame() { return m_frame; };

  private:
    std::string context(const std::string& line, int pos) const;

#ifdef USE_QUEUE
    void process(std::unique_ptr<path_match>& input) final;
#endif

    const data::listview::menu_t m_menu_flags;
    class frame*                 m_frame;
  };
}; // namespace wex::del
