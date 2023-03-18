////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/file.h>
#include <wex/core/path.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>

#include <wex/test/test.h>

namespace wex
{
class file;
};

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

/// Returns the frame.
wex::frame* frame();

/// Returns the statusbar.
wex::statusbar* get_statusbar();

/// Returns an stc.
wex::syntax::stc* get_stc();
