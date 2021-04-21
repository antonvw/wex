////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/link.h>

namespace wex
{
  /// Offers a class holding info about a link, and implementing virtual
  /// methods from base class.
  class link : public factory::link
  {
  public:
    /// Default constructor.
    link();

  private:
    std::string get_link_pairs(const std::string& text) const override;
  };
}; // namespace wex
