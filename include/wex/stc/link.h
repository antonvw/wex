////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/link.h>

#include <string>

namespace wex
{
/// Offers a class holding info about a link, and implementing virtual
/// methods from base class.
class link : public factory::link
{
public:
  /// Default constructor.
  link();

  /// Adds opening link from vcs output text.
  const path get_path(
    /// (vcs) text containing a path somewhere
    const std::string& text,
    /// data to be filled in
    line_data& data,
    /// stc component
    factory::stc* stc = nullptr) override;

private:
  std::string get_link_pairs(const std::string& text) const override;
};
}; // namespace wex
