////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.h
// Purpose:   Declaration of wex::vcs class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wex/vcsentry.h>

namespace wex
{
  class item_dialog;
  class path;

  /// This class collects all vcs handling.
  /// The VCS entries are loaded from menus.xml, this is done
  /// during app startup.
  class vcs
  {
  public:
    /// Default constructor.
    vcs(
      /// Specify several files for which you want vcs action.
      /// Sets the vcs entry for first of the specified files, or
      /// to the base folder if array is empty.
      const std::vector< path > & files = std::vector< path >(),
      /// The command no that is used to set the current vcs command
      /// (index in vcs entry commands).
      int command_no = -1);
    
    /// Shows a dialog allowing you to choose which vcs to use
    /// and to set the path for each vcs entry.
    /// Returns dialog return code.
    int ConfigDialog(const window_data& data = window_data()) const;

    /// Returns true if specified filename (a path) is a vcs directory.
    static bool DirExists(const path& filename);

    /// Executes the current vcs command for the current
    /// vcs entry, and collects the output.
    /// Returns return code from vcs entry Execute.
    bool Execute();
    
    /// Returns branch for current vcs entry, or empty string
    /// if vcs is not used.
    const std::string GetBranch() const;
    
    /// Returns the number of vcs entries.
    static auto GetCount() {return m_Entries.size();};

    /// Returns the current vcs entry.
    const auto& GetEntry() const {return m_Entry;};
    
    /// Returns name for current vcs entry, or empty string
    /// if vcs is not used.
    const std::string GetName() const;
    
    /// Loads all entries (first clears them) from vcs document.
    /// Returns true if document is loaded.
    static bool LoadDocument();

    /// Combines ShowDialog, Execute and vcs entry ShowOutput in one method. 
    /// - Returns wxID_CANCEL if dialog was cancelled, or an execute error occurred.
    /// - Returns wxID_OK if okay (use vcs entry GetError
    ///   to check whether the output contains errors or normal info).
    wxStandardID Request(const window_data& data = window_data());

    /// Sets the vcs entry using base folder.
    /// If not, it will show
    /// a dialog for selecting a vcs folder (if parent is not nullptr).
    /// Returns true if entry is under vcs control.
    bool SetEntryFromBase(
      /// Parent window for showing dir dialog if 
      /// there is not a current directory.
      wxWindow* parent = nullptr);

    /// Shows dialog for the current vcs entry.
    int ShowDialog(const window_data& data = window_data());

    /// Returns true if vcs usage is set in the config.
    bool Use() const;
  private:
    static const vcs_entry FindEntry(const std::string& filename);
    static const vcs_entry FindEntry(const path& filename);
    static const path GetTopLevelDir(
      const std::string& admin_dir, 
      const path& file);
    static bool IsAdminDir(
      const std::string& admin_dir, 
      const path& fn);
    static bool IsAdminDirTopLevel(
      const std::string& admin_dir, 
      const path& fn);

    const path GetFile() const;
    const std::string GetRelativeFile(
      const std::string& admin_dir, 
      const path& file) const;

    vcs_entry m_Entry;

    std::vector< path > m_Files;
    std::string m_Title;

    static std::vector<vcs_entry> m_Entries;
    static item_dialog* m_ItemDialog;
  };
};
