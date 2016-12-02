////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/report/frame.h>

#include "../test.h"

/// Returns the frame.
wxExFrameWithHistory* GetFrame();

/// Returns the project.
const std::string GetProject();
