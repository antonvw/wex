////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../syntax/test.h"

namespace wex
{
namespace test
{

class ctags : public app
{
public:
  static auto* get_stc() { return m_stc; }

  bool OnInit() override;

private:
  inline static test::stc* m_stc = nullptr;
};
}; // namespace test
}; // namespace wex

/// Returns an stc.
wex::syntax::stc* get_stc();
