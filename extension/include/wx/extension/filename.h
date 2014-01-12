////////////////////////////////////////////////////////////////////////////////
// Name:      filename.h
// Purpose:   Declaration of class wxExFileName
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXFILENAME_H
#define _EXFILENAME_H

#include <wx/filename.h>
#include <wx/extension/lexer.h>
#include <wx/extension/stat.h>

class wxExFile;

/// Adds a wxExStat and a wxExLexer member to wxFileName.
class WXDLLIMPEXP_BASE wxExFileName : public wxFileName
{
  friend class wxExFile; // it might update stat
public:
  /// Default constructor.
  wxExFileName() : wxFileName() {;};

  /// Constructor taking a full filename.
  wxExFileName(const wxString& fullpath, wxPathFormat format = wxPATH_NATIVE);

  /// Copy constructor from a wxFileName.
  wxExFileName(const wxFileName& filename);

  /// Assignment operator.
  wxExFileName& operator=(const wxExFileName& f)
  {
    m_Lexer = f.m_Lexer;
    m_Stat = f.m_Stat;
    Assign(f.GetFullPath());
    return *this;
  };

  /// Gets the lexer.
  const wxExLexer& GetLexer() const {return m_Lexer;};

  /// Gets the stat.
  const wxExStat& GetStat() const {return m_Stat;};
  
  /// Sets the lexer.
  void SetLexer(const wxExLexer& lexer) {m_Lexer = lexer;};
private:
  wxExLexer m_Lexer;
  wxExStat m_Stat;
};
#endif
