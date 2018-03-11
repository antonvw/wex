////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.h
// Purpose:   Declaration of class wxExCTags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <wx/extension/ctags-filter.h>
#include <wx/extension/stc-enums.h>
#include <wx/dlimpexp.h>

class wxExCTagsEntry;
class wxExEx;
class wxExFrame;
class wxExSTC;
typedef struct sTagFile tagFile;

/// Offers ctags handling.
class WXDLLIMPEXP_BASE wxExCTags
{
public:  
  /// Constructor, opens ctags file.
  /// Uses ex component for presenting ctags results.
  /// The ctags file is obtained from the STCData associated with ex STC.
  /// Default uses standard ctags file, but you can choose your own name.
  /// This file is searched for in the current dir, and if not found in the 
  /// config dir.
  /// You can also specify an absolute filename.
  wxExCTags(wxExEx* ex);

  /// Constructor, opens default ctags file.
  wxExCTags(wxExFrame* frame);
  
  /// Destructor, closes ctags file.
 ~wxExCTags();
  
  /// Tries to autocomplete text using the tags file.
  /// Returns a string with matches, or empty string if no match is found.
  std::string AutoComplete(
    /// text to be completed
    const std::string& text,
    /// filter on ctags extension field(s), default no filter
    const wxExCTagsFilter& filter = wxExCTagsFilter());

  /// Finds the tag and uses it to fill the supplied filter.
  /// Returns true if a matching tag is found, 
  /// and can be used as a master.
  bool Filter(
    /// tag
    const std::string& name,
    /// filter to be filled
    wxExCTagsFilter& filter) const;

  /// Find the tags matching `name', and fills matches.
  /// Returns true if a matching tag is found,
  /// and calls frame OpenFile if name matches and
  /// there is no next match in another file.
  /// If the name is empty, Next is invoked.
  /// Otherwise shows a dialog to select a file from the matches.
  /// Returns false if dialog was cancelled.
  bool Find(const std::string& name);

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

  wxExEx* m_Ex {nullptr};
  wxExFrame* m_Frame;
  tagFile* m_File {nullptr};
  const int m_Separator;
  bool m_Prepare {false};
  std::map< std::string, wxExCTagsEntry > m_Matches;
  std::map< std::string, wxExCTagsEntry >::iterator m_Iterator;
};
