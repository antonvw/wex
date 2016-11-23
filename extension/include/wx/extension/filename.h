////////////////////////////////////////////////////////////////////////////////
// Name:      filename.h
// Purpose:   Declaration of class wxExFileName
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/filename.h>
#include <wx/extension/lexer.h>
#include <wx/extension/stat.h>

class wxExFile;

/// Offers functionality to handle filenames.
class wxExFileName
{
  friend class wxExFile; // it might update stat
public:
  /// Default constructor.
  wxExFileName() {;};

  /// Constructor taking a fullpath.
  wxExFileName(const std::string& fullpath);

  /// Constructor taking a path and a name.
  wxExFileName(const std::string& path, const std::string& name);

  /// Constructor taking a char array.
  wxExFileName(const char* fullpath);

  /// Constructor from a wxFileName.
  wxExFileName(const wxFileName& filename);
  
  /// == Operator. 
  bool operator==(const wxExFileName& r) const {
    return GetFullPath() == r.GetFullPath();};

  /// != Operator.
  bool operator!=(const wxExFileName& r) const {return !operator==(r);};

  /// Returns true if this file is directory.
  bool DirExists() const {return m_FileName.DirExists();};

  /// Returns true if this file is file.
  bool FileExists() const {return m_FileName.FileExists();};

  /// Returns file extension component.
  const std::string GetExtension() const {return m_FileName.GetExt().ToStdString();};

  /// Returns file fullname (including extension) component.
  const std::string GetFullName() const {return m_FileName.GetFullName().ToStdString();};

  /// Returns file fullpath component (with fullname).
  const std::string GetFullPath(wxPathFormat format = wxPATH_NATIVE) const {
    return m_FileName.GetFullPath(format).ToStdString();};

  /// Returns the lexer.
  const auto & GetLexer() const {return m_Lexer;};

  /// Returns file name component.
  const std::string GetName() const {return m_FileName.GetName().ToStdString();};

  /// Returns file path component (without fullname).
  const std::string GetPath() const {return m_FileName.GetPath().ToStdString();};

  /// Returns the stat.
  const auto & GetStat() const {return m_Stat;};

  /// Returns file volume component.
  const std::string GetVolume() const {return m_FileName.GetVolume().ToStdString();};

  /// Returns true if this filename is initialized.
  bool IsOk() const {return m_FileName.IsOk();};

  /// returns tre if this file (stat) is readonly.
  bool IsReadOnly() const {return m_Stat.IsReadOnly();};
    
  /// Make filename absolute.
  bool MakeAbsolute() {return m_FileName.MakeAbsolute();};
  
  /// Sets the lexer.
  void SetLexer(const wxExLexer& lexer) {m_Lexer = lexer;};
private:
  wxFileName m_FileName;
  wxExLexer m_Lexer;
  wxExStat m_Stat;
};
