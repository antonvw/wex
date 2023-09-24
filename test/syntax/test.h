////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/stc.h>
#include <wex/test/test.h>
#include <wx/app.h>

namespace wex::test
{

/// This class offers a testable syntax::stc by implementing
/// the pure virtual methods.
/// We cannot use the wex::test::stc, as that derives from factory.
class stc : public wex::syntax::stc
{
public:
  stc(const data::window& data = data::window())
    : syntax::stc(data)
  {
    ;
  };

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;
};
}; // namespace wex::test
