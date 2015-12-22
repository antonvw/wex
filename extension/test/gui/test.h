////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../catch.hpp"
#include "../test.h"

class wxExManagedFrame;
class wxExStatusBar;
class wxExShell;
class wxExSTC;

/// Returns abbreviations.
std::vector<std::pair<std::string, std::string>> GetAbbreviations();

/// Returns variables.
std::vector<std::string> GetBuiltinVariables();

/// Returns the frame.
wxExManagedFrame* GetFrame();

/// Returns the statusbar.
wxExStatusBar* GetStatusBar();

/// Returns an STC.
wxExSTC* GetSTC();

/// Processes string on shell.
void Process(const std::string& str, wxExShell* shell);
