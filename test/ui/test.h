////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/stc.h>
#include <wex/file.h>
#include <wex/frame.h>
#include <wex/path.h>

#include "../test.h"

namespace wex
{
class file;
};

class ui_stc : public wex::factory::stc
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
wex::factory::stc* get_stc();
