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
wex::history_frame* GetFrame();

/// Returns the project.
const std::string GetProject();
