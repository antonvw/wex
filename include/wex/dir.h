////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Declaration of class wex::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include <wex/data/dir.h>
#include <wex/interruptible.h>
#include <wex/path.h>
#include <wex/stream-statistics.h>
#include <wex/tool.h>

class wxEvtHandler;

namespace wex
{
/// Offers find_files method.
/// By overriding on_dir and on_file you can take care
/// of what to do with the result.
class dir : public interruptible
{
public:
  /// Returns the statistics.
  static auto& get_statistics() { return m_statistics; }

  /// Constructor.
  dir(
    /// the path to start finding
    const path& path,
    /// the dir data
    const data::dir& data = data::dir(),
    /// event handler to use, results
    /// in using a separate thread for find_files.
    wxEvtHandler* eh = nullptr);

  /// Destructor.
  virtual ~dir() = default;

  /// Virtual interface.

  /// Override for action after find_files has ended.
  virtual void find_files_end() const;

  /// Do something with the dir.
  /// Not made pure virtual, to allow this
  /// class to be tested by calling find_files.
  virtual bool on_dir(const path&) const { return true; }

  /// Do something with the file.
  /// Default does find and replace.
  virtual bool on_file(const path&) const;

  /// Other methods.

  /// Returns the data.
  const auto& data() const { return m_data; }

  /// Finds matching files.
  /// This results in recursive calls for on_dir and on_file.
  /// Runs as a separate thread if event handler is setup,
  /// otherwise runs synchronized.
  /// Returns 1 if thread is started, or number of matches
  /// for synchronized runs.
  int find_files();

  /// Finds matching files, and runs specified tool.
  /// Returns true if thread is started.
  bool find_files(const tool& tool);

  /// Returns the path.
  const auto& get_path() const { return m_dir; }

  /// Returns event handler.
  auto* handler() { return m_eh; }

  /// Returns number of matches.
  auto matches() const;

private:
  int  run() const;
  bool traverse(const std::filesystem::directory_entry& e) const;

  static inline stream_statistics m_statistics;
  const path                      m_dir;
  const data::dir                 m_data;
  wxEvtHandler*                   m_eh{nullptr};
  wex::tool                       m_tool;
};

/// Returns all matching files into a vector of strings (without paths).
std::vector<std::string> get_all_files(
  /// the path to start finding
  const std::string& path,
  /// the dir data
  const data::dir& data = data::dir());

/// Returns all matching files into a vector of paths.
std::vector<path> get_all_files(
  /// the path to start finding
  const path& p,
  /// the dir data
  const data::dir& data = data::dir());
}; // namespace wex
