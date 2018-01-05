////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.h
// Purpose:   Declaration of class wxExCTags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <wx/dlimpexp.h>

class wxExCTagsEntry;
class wxExFrame;
typedef struct sTagFile tagFile;

/// Offers ctags handling.
class WXDLLIMPEXP_BASE wxExCTags
{
public:  
  /// Constructor, opens ctags file.
  wxExCTags(
    /// frame to use for opening source files
    wxExFrame* frame, 
    /// Opens the ctags file on a path.
    /// Default uses standard ctags file, but you can choose your own name.
    /// This file is searched for in the current dir, and if not found in the 
    /// config dir.
    const std::string& filename = "tags");
  
  /// Destructor, closes ctags file.
 ~wxExCTags();
  
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
private:
  wxExFrame* m_Frame;
  tagFile* m_File{nullptr};
  std::map< std::string, wxExCTagsEntry > m_Matches;
  std::map< std::string, wxExCTagsEntry >::iterator m_Iterator;
};
