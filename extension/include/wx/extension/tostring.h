////////////////////////////////////////////////////////////////////////////////
// Name:      tostring.h
// Purpose:   Declaration of specializations of wex::to_container class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <vector>
#include <wx/extension/path.h>
#include <wx/extension/tocontainer.h>

namespace wex
{
  using to_list_string = to_container<std::list < std::string > >;
  using to_vector_path = to_container<std::vector < path > >;
  using to_vector_string = to_container<std::vector < std::string > >;
};
