////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vcs/process.h>

#include "../stc/test.h"
#include "test.h"

namespace wex
{
namespace test
{
class vcs_app : public stc_app
{
public:
  // Virtual interface
  bool OnInit() override;
};
}; // namespace test
}; // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::vcs_app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::vcs_app());
}

bool wex::test::vcs_app::OnInit()
{
  if (!stc_app::OnInit())
  {
    return false;
  }

  process::prepare_output(frame());

  return true;
}
