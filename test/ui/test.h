////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/file.h>
#include <wex/core/path.h>
#include <wex/syntax/stc.h>
#include <wex/test/test.h>
#include <wex/ui/frame.h>

#define ITEM_START()                           \
  auto* panel = new wxScrolledWindow(frame()); \
  frame()->pane_add(panel);                    \
  wex::data::layout layout(panel, 4);          \
  panel->SetSizer(layout.sizer());             \
  panel->SetScrollbars(20, 20, 50, 50);

namespace wex
{
class file;
};

namespace wex::test
{
class ui_stc : public wex::syntax::stc
{
public:
  ui_stc(const wex::data::stc& data = wex::data::stc());

  wex::file& get_file();

  void config_get() { ; }

private:
  const wex::path& path() const override { return m_path; };

  wex::path m_path;
  wex::file m_file;
};
}; // namespace wex::test

/// Returns the frame.
wex::frame* frame();

/// Returns the statusbar.
wex::statusbar* get_statusbar();

/// Returns an stc.
wex::syntax::stc* get_stc();
