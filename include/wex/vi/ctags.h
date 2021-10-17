////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.h
// Purpose:   Declaration of class wex::ctags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/vi/ctags-entry.h>

#include <map>
#include <string>

typedef struct sTagFile tagFile;

namespace wex
{
class ctags_info;
class ex;

const char DEFAULT_TAGFILE[] = "tags";

/// Offers ctags handling.
class ctags
{
public:
  /// Static interface.

  /// Closes ctags file.
  /// Returns false if file was not opened.
  static bool close();

  /// Find the tags matching `tag', and fills the matches container.
  /// Returns true if a matching tag is found,
  /// and calls frame open_file if name matches and
  /// there is no next match in another file.
  /// If the name is empty, next is invoked.
  /// Otherwise shows a dialog to select a file from the matches.
  /// Returns false if dialog was cancelled.
  static bool find(
    /// tag to find
    const std::string& tag,
    /// a possible active ex
    ex* ex = nullptr);

  /// Finds the tag matching 'tag' and uses it to fill the supplied entries.
  /// Returns true if a matching tag is found,
  /// and can be used as a master.
  static bool find(
    /// tag
    const std::string& tag,
    /// tag filter to be filled
    ctags_entry& filter);

  /// Jumps to next match from a previous find.
  static bool next();

  /// Opens ctags file.
  /// Default uses standard ctags file, but you can choose your own name.
  /// This file is searched for in the current dir, and if not found in the
  /// config dir.
  /// You can also specify an absolute filename.
  static void open(const std::string& filename = DEFAULT_TAGFILE);

  /// Jumps to previous match from a previous find.
  static bool previous();

  /// Other methods.

  /// Constructor.
  /// Uses ex component for presenting ctags results,
  /// and opens default tag file, if tag file is not yet opened.
  explicit ctags(ex* ex, bool open = true);

  /// Tries to auto_complete text using the tags file.
  /// Returns a string with matches separated by the
  /// separator character, or empty string if no match is found.
  const std::string auto_complete(
    /// text to be completed
    const std::string& text,
    /// filter on ctags extension entry, default no filter
    const ctags_entry& filter = ctags_entry());

  /// Separator used by auto_complete.
  auto separator() const { return m_separator; }

private:
  void        auto_complete_prepare();
  static bool do_open(const std::string& path);

  ex*       m_ex{nullptr};
  const int m_separator{3};
  bool      m_is_prepared{false};

  static inline tagFile*                             m_file = nullptr;
  static std::map<std::string, ctags_info>           m_matches;
  static std::map<std::string, ctags_info>::iterator m_iterator;
};
}; // namespace wex
