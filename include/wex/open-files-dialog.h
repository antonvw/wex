////////////////////////////////////////////////////////////////////////////////
// Name:      open-files-dialog.h
// Purpose:   Declaration of class wex::open_files_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/dir.h>
#include <wex/data/stc.h>

namespace wex
{
class frame;

/// Shows a dialog and opens selected files
/// (calls open_files).
void open_files_dialog(
  /// frame
  factory::frame* frame,
  /// flags to be used with file_dialog
  bool ask_for_continue = false,
  /// data to be used with open_file
  const data::stc& data = data::stc(),
  /// flags to be used with open_file_dir
  const data::dir::type_t& type = data::dir::type_def());
}; // namespace wex
