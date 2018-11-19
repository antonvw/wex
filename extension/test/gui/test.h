////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../test.h"

namespace wex
{
  class managed_frame;
  class shell;
  class statusbar;
  class stc;
};

/// Returns abbreviations.
std::vector<std::pair<std::string, std::string>> get_abbreviations();

/// Returns variables.
std::vector<std::string> GetBuiltinVariables();

/// Returns the frame.
wex::managed_frame* frame();

/// Returns the statusbar.
wex::statusbar* GetStatusBar();

/// Returns an STC.
wex::stc* get_stc();

/// Processes string on shell.
void Process(const std::string& str, wex::shell* shell);
