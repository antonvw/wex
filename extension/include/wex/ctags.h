////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.h
// Purpose:   Declaration of class wex::ctags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <wex/ctags-entry.h>

typedef struct sTagFile tagFile;

namespace wex
{
  class ctags_info;
  class ex;
  
  const std::string DEFAULT_TAGFILE = "tags";
  
  /// Offers ctags handling.
  class ctags
  {
  public:  
    /// Constructor.
    /// Uses ex component for presenting ctags results.
    ctags(ex* ex, bool open = true);

    /// Tries to autocomplete text using the tags file.
    /// Returns a string with matches separated by the 
    /// separator character, or empty string if no match is found.
    const std::string autocomplete(
      /// text to be completed
      const std::string& text,
      /// filter on ctags extension entry, default no filter
      const ctags_entry& filter = ctags_entry());

    /// Closes ctags file.
    static void close();
    
    /// Find the tags matching `tag', and fills the matches container.
    /// Returns true if a matching tag is found,
    /// and calls frame open_file if name matches and
    /// there is no next match in another file.
    /// If the name is empty, next is invoked.
    /// Otherwise shows a dialog to select a file from the matches.
    /// Returns false if dialog was cancelled.
    static bool find(const std::string& tag);

    /// Finds the tag matching 'tag' and uses it to fill the supplied entries.
    /// Returns true if a matching tag is found, 
    /// and can be used as a master.
    static bool find(
      /// tag
      const std::string& tag,
      /// tag properties to be filled
      ctags_entry& current,
      /// tag filter to be filled
      ctags_entry& filter);

    /// Opens ctags file.
    /// The ctags file is obtained from the STCData associated with ex STC.
    /// Default uses standard ctags file, but you can choose your own name.
    /// This file is searched for in the current dir, and if not found in the 
    /// config dir.
    /// You can also specify an absolute filename.
    static void open(const std::string& filename = DEFAULT_TAGFILE);
    
    /// Jumps to next match from a previous find.
    static bool next();

    /// Jumps to previous match from a previous find.
    static bool previous();

    /// Autocomplete separator.
    auto separator() const {return m_Separator;};
  private:
    void autocomplete_prepare();
    static bool do_open(const std::string& path);

    ex* m_Ex {nullptr};
    const int m_Separator {3};
    bool m_Prepare {false};
    
    static inline tagFile* m_File = nullptr;
    static std::map< std::string, ctags_info > m_Matches;
    static std::map< std::string, ctags_info >::iterator m_Iterator;
  };
};
