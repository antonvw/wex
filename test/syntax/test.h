////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/stc.h>
#include <wex/test/test.h>

namespace wex::test
{

/// This class offers a testable syntax::stc by mocking virtual methods.
class stc : public wex::syntax::stc
{
public:
  stc(const data::window& data = data::window())
    : syntax::stc(data)
  {
    ;
  };

  MAKE_CONST_MOCK0(path, const wex::path&(), override);
};
}; // namespace wex::test
