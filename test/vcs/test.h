////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stc/stc.h>
#include <wex/test/test.h>
#include <wex/ui/frame.h>
#include <wex/vcs/vcs-entry.h>

#include "../stc/test.h"

/// Returns an stc.
wex::stc* get_stc();

/// Returns a git vcs entry from a sample doc.
wex::vcs_entry* load_git_entry();
