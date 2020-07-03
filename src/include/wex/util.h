////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Include file for wex utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <vector>
#include <wex/dir-data.h>
#include <wex/stc-data.h>
#include <wx/combobox.h>

class wxArrayString;
namespace pugi
{
  class xml_node;
  struct xml_parse_result;
}; // namespace pugi

namespace wex
{
  class ex;
  class frame;
  class lexer;
  class path;
  class stc;
  class vcs_command;

  /*! \file */

  /// Adds entries to a combobox from a container.
  template <typename T> void combobox_as(wxComboBox* cb, const T& t)
  {
    if (!t.empty())
    {
      cb->Clear();

      wxArrayString as;
      as.resize(t.size());
      std::copy(t.begin(), t.end(), as.begin());

      cb->Append(as);
      cb->SetValue(cb->GetString(0));
    }
  }

  /// Adds entries to a combobox from a list with strings.
  void combobox_from_list(wxComboBox* cb, const std::list<std::string>& text);

  /// Compares the files, using comparator set in the config.
  bool compare_file(const path& file1, const path& file2);

  /// Shows a dialog with all lexers, allowing you to choose one.
  /// Returns true and sets the lexer on the stc component if you selected
  /// one.
  bool lexers_dialog(stc* stc);

  /// Runs make on specified makefile.
  /// Returns value from executing the make process.
  bool make(
    /// the makefile
    const path& makefile);

  /// Opens all files specified by files.
  /// Returns number of files opened.
  int open_files(
    /// frame on which open_file for each file is called,
    /// and open_file_dir for each dir
    frame* frame,
    /// array with files
    const std::vector<path>& files,
    /// data to be used with open_file
    const data::stc& data = data::stc(),
    /// flags to be used with open_file_dir
    data::dir::type_t type = data::dir::type_t().set());

  /// Shows a dialog and opens selected files
  /// (calls open_files).
  void open_files_dialog(
    /// frame
    frame* frame,
    /// flags to be used with file_dialog
    bool ask_for_continue = false,
    /// data to be used with open_file
    const data::stc& data = data::stc(),
    /// flags to be used with open_file_dir
    data::dir::type_t type = data::dir::type_t().set());

  /// Executes all process between backquotes in command,
  /// and changes command with replaced match with output from process.
  /// Returns false if process could not be executed.
  bool shell_expansion(std::string& command);

  /// Use specified vcs command to set lexer on stc document.
  void vcs_command_stc(
    /// VCS command, used to check for diff or open command
    const vcs_command& command,
    /// lexer to be used
    const lexer& lexer,
    /// stc on which lexer is set
    stc* stc);

  /// Executes VCS command id for specified files
  /// and opens component if necessary.
  void vcs_execute(
    /// frame on which open_file is called
    frame* frame,
    /// VCS menu id to execute
    int id,
    /// files on which to operate
    const std::vector<path>& files);

  /// Shows xml error.
  void xml_error(
    /// xml filename that has error
    const path& filename,
    /// result of parsing describing the error
    const pugi::xml_parse_result* result,
    /// stc component containing the filename
    stc* stc = nullptr);
}; // namespace wex
