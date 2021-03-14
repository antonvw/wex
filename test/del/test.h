////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/del/frame.h>

#include "../test.h"

/// Returns the frame.
wex::del::frame* del_frame();

/// Returns the project.
const std::string get_project();
