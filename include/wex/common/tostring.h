////////////////////////////////////////////////////////////////////////////////
// Name:      tostring.h
// Purpose:   Declaration of specializations of wex::to_container class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/common/tocontainer.h>
#include <wex/core/path.h>
#include <wex/core/types.h>

namespace wex
{
using to_list_string   = to_container<strings_t>;
using to_vector_path   = to_container<std::vector<path>>;
using to_vector_string = to_container<std::vector<std::string>>;
}; // namespace wex
