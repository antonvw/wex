////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/stc.h>
#include <wx/app.h>

/// Add general test header.
#include "../test.h"

namespace wex::test
{

/// This class offers a testable syntax::stc by implementing
/// the pure virtual methods.
/// We cannot use the wex::test::stc, as that derives from factory.
class stc : public wex::syntax::stc
{
public:
  stc(wxFrame* parent = nullptr)
  {
    if (parent != nullptr)
    {
      Create(parent, -1);
    }
    else if (wxTheApp != nullptr)
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
