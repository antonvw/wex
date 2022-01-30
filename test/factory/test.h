////////////////////////////////////////////////////////////////////////////////
// Name:      factory/test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/stc.h>

#include "../test.h"

namespace wex::test
{

/// This class offers a testable factory::stc by implementing
/// the pure virtual methods.
class stc : public wex::factory::stc
{
public:
  stc()
  {
    if (wxTheApp != nullptr)
    {
      Create(wxTheApp->GetTopWindow(), -1);
    }
    else
    {
      std::cout << "no parent available\n";
    }
  };

  bool is_visual() const override { return m_visual; }
  void visual(bool on) override { m_visual = on; }

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;

  bool m_visual{true};
};
}; // namespace wex::test
