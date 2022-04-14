////////////////////////////////////////////////////////////////////////////////
// Name:      type-to-value.cpp
// Purpose:   Implementation of class wex::type_to_value
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/core/type-to-value.h>

void wex::log_type_to_value_error(
  const std::exception& e,
  const std::string&    text)
{
  log(e) << "value:" << text;
}
