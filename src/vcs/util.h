////////////////////////////////////////////////////////////////////////////////
// Name:      stc/util.h
// Purpose:   Declaration of util methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/process-data.h>

namespace wex
{
class stc;

void expand_macro(wex::process_data& data, stc* stc);
}; // namespace wex
