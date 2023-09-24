////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/frame.h>
#include <wex/factory/stc.h>
#include <wex/test/test.h>

namespace wex::test
{

/// This class offers a testable factory::stc by implementing
/// the pure virtual methods.
class stc : public wex::factory::stc
{
public:
  /// Default constructor.
  stc()
    : wex::factory::stc()
  {
    ;
  };

  /// Overriden virtual methods.
  bool is_visual() const override { return m_visual; }
  void visual(bool on) override { m_visual = on; }

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;

  bool m_visual{true};
};
}; // namespace wex::test

/// Returns the frame.
wex::factory::frame* frame();
