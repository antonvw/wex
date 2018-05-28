////////////////////////////////////////////////////////////////////////////////
// Name:      path.h
// Purpose:   Declaration of class wxExPath
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
  /// If path is empty, it saves the current path, and when destructed restores it to current.
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

  /// Destructor.
 ~wxExPath();

  /// == Operator. 
  bool operator==(const wxExPath& r) const {return Path() == r.Path();};

  /// != Operator.
  bool operator!=(const wxExPath& r) const {return !operator==(r);};

  /// Appends path.
  wxExPath& Append(const wxExPath& path);

  /// Normalizes the path.
  /// Returns false if the path does not exist.
  bool Canonical(const std::string& path);

  /// Returns current path.
  static std::string Current() {
    return std::experimental::filesystem::current_path().string();};

  /// Sets current path.
  static void Current(const std::string& path);

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

  /// Returns original path.
  const auto & GetOriginal() {return m_path_original;};

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

  /// Opens this path using registered mime type.
  /// Returns false if no mime type is found.
  bool OpenMIME() const;
  
  /// Returns path.
  /// E.g. Path().string() returns fullpath.
  // (cannot be auto)
  const std::experimental::filesystem::path& Path() const {return m_path;};
    
  /// Replaces filename.
  wxExPath& ReplaceFileName(const std::string& filename);
private:
  std::experimental::filesystem::path m_path;
  std::string m_path_original;
  wxExLexer m_Lexer;
  wxExStat m_Stat;
};
