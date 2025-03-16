////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/test/test.h>

namespace wex
{
namespace test
{

class ctags : public app
{
public:
  bool OnInit() override;
};
}; // namespace test
}; // namespace wex
