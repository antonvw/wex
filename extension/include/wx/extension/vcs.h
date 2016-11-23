////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.h
// Purpose:   Declaration of wxExVCS class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/extension/vcsentry.h>

class wxExFileName;

/// This class collects all vcs handling.
/// The VCS entries are loaded from menus.xml, this is done
/// during wxExApp startup.
class WXDLLIMPEXP_BASE wxExVCS
{
public:
  /// Default constructor.
  wxExVCS(
    /// Specify several files for which you want vcs action.
    /// Sets the vcs entry for first of the specified files, or
    /// to the base folder if array is empty.
    const std::vector< std::string > & files = std::vector< std::string >(),
    /// The command no that is used to set the current vcs command
    /// (index in vcs entry commands).
    int command_no = -1);
  
#if wxUSE_GUI
  /// Shows a dialog allowing you to choose which vcs to use
  /// and to set the path for each vcs entry.
  /// Returns dialog return code.
  int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Set VCS"),
    bool modal = true) const;
#endif    

  /// Returns true if specified filename (a path) is a vcs directory.
  static bool DirExists(const wxExFileName& filename);

  /// Executes the current vcs command for the current
  /// vcs entry, and collects the output.
  /// Returns return code from vcs entry Execute.
  bool Execute();
  
  /// Returns the number of vcs entries.
  static int GetCount() {return m_Entries.size();};

  /// Returns the current vcs entry.
  const auto& GetEntry() const {return m_Entry;};
  
  /// Returns name for current vcs entry, or empty string
  /// if vcs is not used.
  const std::string GetName() const;
  
  /// Loads all entries (first clears them) from vcs document.
  /// Returns true if document is loaded.
  static bool LoadDocument();

#if wxUSE_GUI
  /// Combines ShowDialog, Execute and vcs entry ShowOutput in one method. 
  /// - Returns wxID_CANCEL if dialog was cancelled, or an execute error occurred.
  /// - Returns wxID_OK if okay (use vcs entry GetError
  ///   to check whether the output contains errors or normal info).
  wxStandardID Request(wxWindow* parent);
#endif  

  /// Sets the vcs entry using base folder.
  /// If not, it will show
  /// a dialog for selecting a vcs folder (if parent is not nullptr).
  /// Returns true if entry is under vcs control.
  bool SetEntryFromBase(
    /// Parent window for showing dir dialog if 
    /// there is not a current directory.
    wxWindow* parent = nullptr);

#if wxUSE_GUI
  /// Calls show dialog for the current vcs entry.
  int ShowDialog(wxWindow* parent) const {
    return m_Entry.ShowDialog(parent, m_Caption, m_Files.empty());};
#endif

  /// Returns true if vcs usage is set in the config.
  bool Use() const;
private:
  static const wxExVCSEntry FindEntry(const std::string& filename);
  static const wxExVCSEntry FindEntry(const wxExFileName& filename);
  static bool IsAdminDir(
    const std::string& admin_dir, 
    const wxExFileName& fn);
  static bool IsAdminDirTopLevel(
    const std::string& admin_dir, 
    const wxExFileName& fn);
  
  const std::string GetFile() const;
  const std::string GetRelativeFile(
    const std::string& admin_dir, 
    const wxExFileName& file) const;
  const std::string GetTopLevelDir(
    const std::string& admin_dir, 
    const wxExFileName& file) const;
  
  wxExVCSEntry m_Entry;

  std::vector< std::string > m_Files;
  wxString m_Caption;

  static std::vector<wxExVCSEntry> m_Entries;
};
