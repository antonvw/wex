////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Include file for wex utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/common/path-match.h>
#include <wex/core/core.h>
#include <wex/core/types.h>
#include <wex/data/dir.h>
#include <wex/data/stc.h>
#include <wx/combobox.h>
#include <wx/listbox.h>

#include <optional>
#include <vector>

namespace pugi
{
struct xml_parse_result;
}; // namespace pugi

class wxArrayString;

namespace wex
{
class ex;
class lexer;
class path;
class vcs_command;

namespace factory
{
class frame;
}; // namespace factory

namespace syntax
{
class stc;
};

/*! \file */

typedef struct
{
  /// expansion of text to matching filename
  /// (if only 1 match exists)
  /// or common part of matching filenames
  std::string expansion;
  /// vector containing completed file name(s)
  std::vector<std::string> vector;
} auto_complete_filename_t;

/// Tries to auto complete filename,
/// the result is stored as a auto_complete_filename_t.
std::optional<auto_complete_filename_t> auto_complete_filename(
  /// text containing start of a filename
  const std::string& text);

/// Sets entries for a combobox from a container.
template <typename T> void combobox_as(wxComboBox* cb, const T& t)
{
  cb->Clear();

  if (!t.empty())
  {
    wxArrayString as;
    as.resize(t.size());
    std::copy(t.begin(), t.end(), as.begin());

    cb->Append(as);
    cb->SetValue(cb->GetString(0));
  }
  else
  {
    cb->SetValue(std::string());
  }
}

/// Sets entries for a combobox from a list with strings.
void combobox_from_list(wxComboBox* cb, const strings_t& text);

/// Compares the files, using comparator set in the config.
bool compare_file(const path& file1, const path& file2);

/// Shows a dialog with all lexers, allowing you to choose one.
/// Returns true and sets the lexer on the stc component if you selected
/// one.
bool lexers_dialog(syntax::stc* stc);

/// Sets entries for a listbox from a container.
template <typename T> void listbox_as(wxListBox* lb, const T& t)
{
  lb->Clear();

  if (!t.empty())
  {
    int i = 0;

    for (const auto& item : t)
    {
      lb->Append(find_before(item, ":"));

      if (find_after(item, ":") == "1")
      {
        lb->SetSelection(i);
      }

      i++;
    }
  }
}

/// Opens all files specified by files.
/// Returns number of files opened.
int open_files(
  /// frame on which open_file for each file is called,
  /// and open_file_dir for each dir
  factory::frame* frame,
  /// array with files
  const std::vector<path>& files,
  /// data to be used with open_file
  const data::stc& data = data::stc(),
  /// flags to be used with open_file_dir
  const data::dir::type_t& type = data::dir::type_t_def());

/// Processes a match.
void process_match(
  /// the match path
  const path_match& m,
  ///  the event handler that will receive the match (ID_LIST_MATCH)
  wxEvtHandler* eh);

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
  syntax::stc* stc);

/// Shows xml error.
void xml_error(
  /// xml filename that has error
  const path& filename,
  /// result of parsing describing the error
  const pugi::xml_parse_result* result,
  /// stc component containing the filename
  syntax::stc* stc = nullptr);
}; // namespace wex
