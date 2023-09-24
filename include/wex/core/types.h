////////////////////////////////////////////////////////////////////////////////
// Name:      types.h
// Purpose:   Declaration of core types
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <string>
#include <vector>

namespace wex
{
/// Type for keeping string values.
typedef std::list<std::string> strings_t;

/// Type for keeping int values.
typedef std::vector<int> ints_t;
} // namespace wex
