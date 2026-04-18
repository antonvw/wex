////////////////////////////////////////////////////////////////////////////////
// Name:      vcs/util.h
// Purpose:   Declaration of util methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/process-data.h>

namespace wex
{
class path;
class stc;
class vcs_entry;

/// Runs vcs bin grep in specified toplevel dir.
/// Requires an stc component on the main frame, if text is selected
/// greps for that text, otherwise uses stc_entry_dialog for input.
/// Returns false if error occurred or dialog canceled.
bool execute_grep(
  /// the binary
  const std::string& bin,
  /// toplevel path where to grep
  const path& toplevel);

/// Expands a %LINES macro in process_data exe.
/// Return false if no such macro is present.
bool expand_macro(wex::process_data& data, stc* stc);

/// Returns path specs from file types.
/// E.g. *.h;*.cpp -> -- "*.h" "*.cpp"
std::string path_spec(const std::string& extentions);

/// Returns true if specified vcs_entry supports (unified) diff,
/// the command is a diff command, and config Use unified diff view is set.
bool vcs_diff(const vcs_entry& e, const std::string& command);
}; // namespace wex
