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
  class frame;
  
  /// Offers ctags handling.
  class ctags
  {
  public:  
    /// Constructor, opens ctags file.
    /// Uses ex component for presenting ctags results.
    /// The ctags file is obtained from the STCData associated with ex STC.
    /// Default uses standard ctags file, but you can choose your own name.
    /// This file is searched for in the current dir, and if not found in the 
    /// config dir.
    /// You can also specify an absolute filename.
    ctags(ex* ex);

    /// Constructor, opens default ctags file.
    ctags(frame* frame);
    
    /// Destructor, closes ctags file.
   ~ctags();
    
    /// Tries to autocomplete text using the tags file.
    /// Returns a string with matches, or empty string if no match is found.
    std::string AutoComplete(
      /// text to be completed
      const std::string& text,
      /// filter on ctags extension entry, default no filter
      const ctags_entry& filter = ctags_entry());

    /// Find the tags matching `tag', and fills matches.
    /// Returns true if a matching tag is found,
    /// and calls frame OpenFile if name matches and
    /// there is no next match in another file.
    /// If the name is empty, Next is invoked.
    /// Otherwise shows a dialog to select a file from the matches.
    /// Returns false if dialog was cancelled.
    bool Find(const std::string& tag);

    /// Finds the tag matching 'tag' and uses it to fill the supplied entries.
    /// Returns true if a matching tag is found, 
    /// and can be used as a master.
    bool Find(
      /// tag
      const std::string& tag,
      /// tag properties to be filled
      ctags_entry& current,
      /// tag filter to be filled
      ctags_entry& filter) const;

    /// Jumps to next match from a previous Find.
    bool Next();

    /// Jumps to previous match from a previous Find.
    bool Previous();

    /// Autocomplete separator.
    auto Separator() const {return m_Separator;};
  private:
    void AutoCompletePrepare();
    void Init(const std::string& filename);
    bool Open(const std::string& path, bool show_error = false);

    ex* m_Ex {nullptr};
    frame* m_Frame;
    tagFile* m_File {nullptr};
    const int m_Separator {3};
    bool m_Prepare {false};
    static std::map< std::string, ctags_info > m_Matches;
    static std::map< std::string, ctags_info >::iterator m_Iterator;
  };
};
