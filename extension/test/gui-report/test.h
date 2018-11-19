////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/report/frame.h>

#include "../test.h"

/// Returns the frame.
wex::history_frame* frame();

/// Returns the project.
const std::string get_project();
