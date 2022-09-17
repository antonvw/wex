////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/stc.h>
#include <wx/app.h>

#include "../test.h"

namespace wex::test
{

/// This class offers a testable syntax::stc by implementing
/// the pure virtual methods.
class stc : public wex::syntax::stc
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

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;
};
}; // namespace wex::test
