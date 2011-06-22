////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.h
// Purpose:   Declaration of wxExVCS class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVCS_H
#define _EXVCS_H

#include <map>
#include <wx/filename.h>
#include <wx/extension/vcsentry.h>

/// This class collects all vcs handling.
/// The VCS entries are read in from vcs.xml, this is done
/// during wxExApp startup.
class WXDLLIMPEXP_BASE wxExVCS
{
public:
  /// Default constructor.
  wxExVCS(
    /// Specify several files for which you want vcs action.
    /// If this array is empty, ShowDialog will show
    /// a combobox for selecting a vcs folder.
    const wxArrayString& files = wxArrayString(),
    /// The menu command id that is used to set the vcs command.
    int menu_id = -1,
    /// Parent window for showing dir dialog if 
    /// there is not a current directory.
    wxWindow* parent = NULL);
  
  /// Constructor for vcs from specified filename.
  wxExVCS(
    /// This must be an existing xml file containing all vcs.
    /// This is done during wxExApp startup.
    const wxFileName& filename) {m_FileName = filename;};

#if wxUSE_GUI
  /// Shows a dialog allowing you to choose which vcs to use
  /// and to set the path for each known vcs.
  /// Returns dialog return code.
  int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Set VCS")) const;
#endif    

  /// Returns true if specified filename (a path) is a vcs directory.
  static bool DirExists(const wxFileName& filename);

  /// Executes the vcs command, and collects the output.
  /// Returns return code from command Execute.
  long Execute();
  
  /// Gets the number of vcs entries.
  static int GetCount() {return m_Entries.size();};

  /// Gets the current vcs entry.
  const wxExVCSEntry& GetEntry() const {return m_Entry;};
  
  /// Gets the xml filename.
  static const wxFileName& GetFileName() {return m_FileName;};
  
  /// Returns name for current vcs entry, or empty string
  /// if vcs is not used.
  const wxString GetName() const;
  
  /// Reads all vcs (first clears them) from file.
  /// Returns true if the file could be read and loaded as valid xml file.
  static bool Read();

#if wxUSE_GUI
  /// Combines ShowDialog, Execute and vcs entry ShowOutput in one method. 
  /// Returns wxID_CANCEL if dialog was cancelled, or an execute error occurred.
  /// Returns wxID_OK if okay (use vcs entry GetError
  /// to check whether the output contains errors or normal info).
  wxStandardID Request(wxWindow* parent);
#endif  

#if wxUSE_GUI
  /// Calls show dialog for the selected vcs entry.
  int ShowDialog(wxWindow* parent) const {
    return m_Entry.ShowDialog(parent, m_Caption, m_Files.empty());};
#endif

  /// Returns true if VCS usage is set in the config.
  bool Use() const;
private:
  // The vcs id's here can be set using the config dialog, and are not
  // present in the vcs.xml. 
  enum
  {
    VCS_NONE = 0, // no version control
    VCS_AUTO, // uses the VCS appropriate for current file
    VCS_MAX
  };

  static bool CheckPath(const wxString& vcs, const wxFileName& fn);
  static bool CheckPathAll(const wxString& vcs, const wxFileName& fn);
  static const wxExVCSEntry FindEntry(const wxFileName& filename);
  const wxString GetFile(wxWindow* parent = NULL) const;
  static bool IsCheckPathAllVCS(const wxString& vcs);
  
  wxExVCSEntry m_Entry;

  wxArrayString m_Files;
  wxString m_Caption;

  static std::map<wxString, wxExVCSEntry> m_Entries;
  static wxFileName m_FileName;
};
#endif
