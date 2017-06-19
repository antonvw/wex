////////////////////////////////////////////////////////////////////////////////
// Name:      path.h
// Purpose:   Declaration of class wxExPath
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <wx/extension/lexer.h>
#include <wx/extension/stat.h>

class wxExFile;
class wxExPathImp;

/// Offers functionality to handle paths.
class wxExPath
{
  friend class wxExFile; // it might update stat
public:
  /// Default constructor taking a fullpath.
  wxExPath(const std::string& fullpath = std::string());

  /// Constructor taking a path and a name.
  wxExPath(const std::string& path, const std::string& name);

  /// Constructor taking a char array.
  wxExPath(const char* fullpath);

  /// Constructor from a vector of paths.
  wxExPath(const std::vector<std::string> v);

  /// Copy constructor.
  wxExPath(const wxExPath& r);
  
  /// Destructor.
 ~wxExPath();

  /// Assignment operator.
  wxExPath& operator=(const wxExPath& r);

  /// == Operator. 
  bool operator==(const wxExPath& r) const {
    return GetFullPath() == r.GetFullPath();};

  /// != Operator.
  bool operator!=(const wxExPath& r) const {return !operator==(r);};

  /// Appends path.
  wxExPath& Append(const wxExPath& path);

  /// Normalizes the path.
  /// Returns false if the path does not exist.
  bool Canonical(const std::string& path);

  /// Returns true if the directory with this name exists.
  bool DirExists() const;

  /// Returns true if the file with this name exists.
  bool FileExists() const;

  /// Returns path extension component (including the .).
  const std::string GetExtension() const;

  /// Returns path fullname (including extension) component.
  const std::string GetFullName() const;

  /// Returns path fullpath component (with fullname).
  const std::string GetFullPath() const;

  /// Returns the lexer.
  const auto & GetLexer() const {return m_Lexer;};

  /// Returns path name component.
  const std::string GetName() const;

  /// Returns path path component (without fullname).
  const std::string GetPath() const;

  /// Returns path components.
  const std::vector<wxExPath> GetPaths() const;

  /// Returns the stat.
  const auto & GetStat() const {return m_Stat;};

  /// Returns path volume component.
  const std::string GetVolume() const;

  /// Returns true if this path is absolute.
  bool IsAbsolute() const;
    
  /// Returns true if this path (stat) is readonly.
  bool IsReadOnly() const;
    
  /// Returns true if this path is relative.
  bool IsRelative() const;
    
  /// Make this path absolute.
  wxExPath& MakeAbsolute(const wxExPath& base = wxExPath());
  
  /// Replaces filename.
  wxExPath& ReplaceFileName(const std::string& filename);
private:
  std::unique_ptr<wxExPathImp> m_Path;
  wxExLexer m_Lexer;
  wxExStat m_Stat;
};
