////////////////////////////////////////////////////////////////////////////////
// Name:      filename.h
// Purpose:   Declaration of class wxExFileName
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/filename.h>
#include <wx/extension/lexer.h>
#include <wx/extension/lexers.h>
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
  wxExFileName(const wxString& fullpath, wxPathFormat format = wxPATH_NATIVE)
    : wxFileName(fullpath, format)
    , m_Stat(fullpath) {m_Lexer = wxExLexers::Get()->FindByFileName(*this);};

  /// Copy constructor.
  wxExFileName(const wxExFileName& f)
    : wxFileName(f) {*this = f;};

  /// Copy constructor from a wxFileName.
  wxExFileName(const wxFileName& filename)
    : wxExFileName(filename.GetFullPath()) {;};

  /// Assignment operator.
  wxExFileName& operator=(const wxExFileName& f)
  {
    if (this != &f) {
      m_Lexer = f.m_Lexer;
      m_Stat = f.m_Stat;
      Assign(f.GetFullPath());};
    return *this;
  };

  /// Returns the lexer.
  const auto & GetLexer() const {return m_Lexer;};

  /// Returns the stat.
  const auto & GetStat() const {return m_Stat;};
  
  /// Sets the lexer.
  void SetLexer(const wxExLexer& lexer) {m_Lexer = lexer;};
private:
  wxExLexer m_Lexer;
  wxExStat m_Stat;
};
