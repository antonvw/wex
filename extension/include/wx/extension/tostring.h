////////////////////////////////////////////////////////////////////////////////
// Name:      tostring.h
// Purpose:   Declaration of specializations of wxExToContainer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <vector>
#include <wx/extension/path.h>
#include <wx/extension/tocontainer.h>

using wxExToListString = wxExToContainer<std::list < std::string > >;
using wxExToVectorPath = wxExToContainer<std::vector < wxExPath > >;
using wxExToVectorString = wxExToContainer<std::vector < std::string > >;
