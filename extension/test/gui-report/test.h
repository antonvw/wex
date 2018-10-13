////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wex::tension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/report/frame.h>

#include "../test.h"

/// Returns the frame.
wex::history_frame* GetFrame();

/// Returns the project.
const std::string GetProject();
