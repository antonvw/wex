////////////////////////////////////////////////////////////////////////////////
// Name:      path.h
// Purpose:   Declaration of class wxExPath
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <experimental/filesystem>
#include <string>
#include <vector>
#include <wx/extension/lexer.h>
#include <wx/extension/stat.h>

class wxExFile;

/// Offers functionality to handle paths.
class wxExPath
{
  friend class wxExFile; // it might update stat
public:
  /// Default constructor taking a path.
  wxExPath(const std::experimental::filesystem::path& p = std::experimental::filesystem::path());

  /// Constructor using string path.
  wxExPath(const std::string& path);

  /// Constructor using a path and a name.
  wxExPath(const std::string& path, const std::string& name);

  /// Constructor using a char array.
  wxExPath(const char* path);

  /// Constructor from a vector of paths.
  wxExPath(const std::vector<std::string> v);

  /// Copy constructor.
  wxExPath(const wxExPath& r);
  
  /// Assignment operator.
  wxExPath& operator=(const wxExPath& r);

  /// == Operator. 
  bool operator==(const wxExPath& r) const {return Path() == r.Path();};

  /// != Operator.
  bool operator!=(const wxExPath& r) const {return !operator==(r);};

  /// Appends path.
  wxExPath& Append(const wxExPath& path);

  /// Normalizes the path.
  /// Returns false if the path does not exist.
  bool Canonical(const std::string& path);

  /// Returns true if the directory with this name exists.
  bool DirExists() const {
    return std::experimental::filesystem::is_directory(m_path);};

  /// Returns true if the file with this name exists.
  bool FileExists() const {
    return std::experimental::filesystem::is_regular_file(m_path);};

  /// Returns path extension component (including the .).
  const std::string GetExtension() const {
    return m_path.extension().string();};

  /// Returns path fullname (including extension) component.
  const std::string GetFullName() const {
    return m_path.filename().string();};

  /// Returns the lexer.
  const auto & GetLexer() const {return m_Lexer;};

  /// Returns path name component.
  const std::string GetName() const {
    return m_path.stem().string();};

  /// Returns path path component (without fullname).
  const std::string GetPath() const {
    return m_path.parent_path().string();};

  /// Returns path components.
  const std::vector<wxExPath> GetPaths() const;

  /// Returns the stat.
  const auto & GetStat() const {return m_Stat;};

  /// Returns true if this path is absolute.
  bool IsAbsolute() const {return m_path.is_absolute();};
  
  /// Returns true if this path (stat) is readonly.
  bool IsReadOnly() const {return m_Stat.IsReadOnly();};

  /// Returns true if this path is relative.
  bool IsRelative() const {return m_path.is_relative();};
    
  /// Make this path absolute.
  wxExPath& MakeAbsolute(const wxExPath& base = wxExPath());
  
  /// Returns path.
  /// E.g. Path().string() returns fullpath.
  const std::experimental::filesystem::path& Path() const {return m_path;};
    
  /// Replaces filename.
  wxExPath& ReplaceFileName(const std::string& filename);
private:
  std::experimental::filesystem::path m_path;
  wxExLexer m_Lexer;
  wxExStat m_Stat;
};
