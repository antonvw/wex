////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.h
// Purpose:   Declaration of wxExVCS class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVCS_H
#define _EXVCS_H

#include <vector>
#include <wx/filename.h>
#include <wx/extension/vcsentry.h>

/// This class collects all vcs handling.
/// The VCS entries are loaded from vcs.xml, this is done
/// during wxExApp startup.
class WXDLLIMPEXP_BASE wxExVCS
{
public:
  /// Default constructor.
  wxExVCS(
    /// Specify several files for which you want vcs action.
    /// Sets vcs entry for first of the specified files, or
    /// to the base folder if array is empty.
    const wxArrayString& files = wxArrayString(),
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
  static bool DirExists(const wxFileName& filename);

  /// Executes the current vcs command, and collects the output.
  /// Returns return code from vcs entry Execute.
  bool Execute();
  
  /// Gets the number of vcs entries.
  static int GetCount() {return m_Entries.size();};

  /// Gets the current vcs entry.
  const wxExVCSEntry& GetEntry() const {return m_Entry;};
  
  /// Gets the xml filename.
  static const wxFileName GetFileName();
  
  /// Returns name for current vcs entry, or empty string
  /// if vcs is not used.
  const wxString GetName() const;
  
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

  /// Sets the entry using base folder.
  /// If not, it will show
  /// a dialog for selecting a vcs folder (if parent is not NULL).
  /// Returns true if entry is under vcs control.
  bool SetEntryFromBase(
    /// Parent window for showing dir dialog if 
    /// there is not a current directory.
    wxWindow* parent = NULL);

#if wxUSE_GUI
  /// Calls show dialog for the current vcs entry.
  int ShowDialog(wxWindow* parent) const {
    return m_Entry.ShowDialog(parent, m_Caption, m_Files.empty());};
#endif

  /// Returns true if VCS usage is set in the config.
  bool Use() const;
private:
  static const wxExVCSEntry FindEntry(const wxFileName& filename);
  static bool IsAdminDir(
    const wxString& admin_dir, 
    const wxFileName& fn);
  static bool IsAdminDirTopLevel(
    const wxString& admin_dir, 
    const wxFileName& fn);
  
  const wxString GetFile() const;
  const wxString GetRelativeFile(
    const wxString& admin_dir, 
    const wxFileName& file) const;
  const wxString GetTopLevelDir(
    const wxString& admin_dir, 
    const wxFileName& file) const;
  
  wxExVCSEntry m_Entry;

  wxArrayString m_Files;
  wxString m_Caption;

  static std::vector<wxExVCSEntry> m_Entries;
};
#endif
