////////////////////////////////////////////////////////////////////////////////
// Name:      item-vector.cpp
// Purpose:   Implementtion of wex::item_vector class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/ui/item-vector.h>

void wex::item_vector::log_error(const std::string& text) const
{
  log("unknown item") << text;
}

void wex::item_vector::log_error(
  const std::string&    text,
  const std::exception& e) const
{
  log(e) << text;
}
