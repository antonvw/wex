////////////////////////////////////////////////////////////////////////////////
// Name:      path.h
// Purpose:   Declaration of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <wex/lexer.h>
#include <wex/stat.h>

namespace wex
{
  class file;

  /// Offers functionality to handle paths.
  class path
  {
    friend class file; // it might update file_stat
  public:
    /// Default constructor taking a path.
    /// If path is empty, it saves the current path, 
    /// and when destructed restores it to current.
    path(const std::filesystem::path& p = std::filesystem::path());

    /// Constructor using string path.
    path(const std::string& path);

    /// Constructor using a path and a name.
    path(const std::string& path, const std::string& name);

    /// Constructor using a char array.
    path(const char* path);

    /// Constructor from a vector of paths.
    path(const std::vector<std::string> & v);

    /// Copy constructor.
    path(const path& r);
    
    /// Assignment operator.
    path& operator=(const path& r);

    /// Destructor.
   ~path();

    /// == Operator. 
    bool operator==(const path& r) const {return Path() == r.Path();};

    /// != Operator.
    bool operator!=(const path& r) const {return !operator==(r);};

    /// Appends path.
    path& Append(const path& path);

    /// Returns current path.
    static std::string Current() {
      return std::filesystem::current_path().string();};

    /// Sets current path.
    static void Current(const std::string& path);

    /// Returns true if the directory with this name exists.
    bool DirExists() const {
      return std::filesystem::is_directory(m_path);};

    /// Returns true if the file with this name exists.
    bool FileExists() const {
      return std::filesystem::is_regular_file(m_path);};

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
    const auto & original() {return m_path_original;};

    /// Returns path path component (without fullname).
    const std::string GetPath() const {
      return m_path.parent_path().string();};

    /// Returns path components.
    const std::vector<path> paths() const;

    /// Returns the stat.
    const auto & GetStat() const {return m_Stat;};

    /// Returns true if this path is absolute.
    bool is_absolute() const {return m_path.is_absolute();};
    
    /// Returns true if this path (stat) is readonly.
    bool IsReadOnly() const {return m_Stat.is_readonly();};

    /// Returns true if this path is relative.
    bool is_relative() const {return m_path.is_relative();};
      
    /// Make this path absolute.
    path& make_absolute();

    /// Opens this path using registered mime type.
    /// Returns false if no mime type is found.
    bool open_mime() const;
    
    /// Returns path.
    /// E.g. Path().string() returns fullpath.
    // (cannot be auto)
    const std::filesystem::path& Path() const {return m_path;};
      
    /// Replaces filename.
    path& replace_filename(const std::string& filename);
  private:
    std::filesystem::path m_path;
    std::string m_path_original;
    lexer m_Lexer;
    file_stat m_Stat;
  };
};
