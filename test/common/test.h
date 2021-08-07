////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/frame.h>
#include <wex/factory/listview.h>
#include <wex/factory/stc.h>

#include "../test.h"

wex::factory::frame*    get_frame();
wex::factory::listview* get_listview();
wex::factory::stc*      get_stc();
