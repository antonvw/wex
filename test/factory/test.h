////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/frame.h>
#include <wex/factory/stc.h>
#include <wex/test/test.h>

namespace wex::test
{

/// This class offers a testable factory::stc by mocking virtual methods.
class stc : public wex::factory::stc
{
public:
  /// Default constructor.
  stc() = default;

  MAKE_CONST_MOCK0(is_visual, bool(), override);
  MAKE_CONST_MOCK0(path, const wex::path&(), override);
  MAKE_MOCK1(visual, void(bool), override);
};
}; // namespace wex::test

/// Returns the frame.
wex::factory::frame* frame();
