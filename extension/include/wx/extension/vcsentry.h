////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.h
// Purpose:   Declaration of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVCSENTRY_H
#define _EXVCSENTRY_H

#include <vector>
#include <wx/menu.h>
#include <wx/xml/xml.h>
#include <wx/extension/lexer.h>
#include <wx/extension/process.h>
#include <wx/extension/vcscommand.h>

/// This class collects a single vcs.
class WXDLLIMPEXP_BASE wxExVCSEntry : public wxExProcess
{
public:
  /// Default constructor.
  /// Adds empty vcs command with id 0.
  wxExVCSEntry();
  
  /// Constructor using xml node.
  wxExVCSEntry(const wxXmlNode* node);

  /// Returns true if admin dir is only at top level.
  bool AdminDirIsTopLevel() const {return m_AdminDirIsTopLevel;};

#if wxUSE_GUI
  /// Builds a menu from all vcs commands.
  /// Returns (total) number of items in menu.
  int BuildMenu(
    /// menu id to be added to the vcs commands
    int base_id, 
    /// menu to be built
    wxMenu* menu, 
    /// default assumes this is a popup menu
    bool is_popup = true) const;
#endif

  /// Executes the current vcs command.
  /// Might ask for vcs binary if it is not yet known.
  /// Return code is code from process Execute,
  /// and also can be false if dialog for vcs bin was cancelled.
  bool Execute(
    /// args
    const wxString& args,
    /// lexer that is used for presenting the output
    const wxExLexer& lexer,
    /// working directory
    const wxString& wd = wxEmptyString);
  
  /// Gets the administrative directory.
  const wxString& GetAdminDir() const {return m_AdminDir;};

  /// Gets the current vcs command.  
  const wxExVCSCommand& GetCommand() const {
    return m_Commands.at(m_CommandIndex);};

  /// Gets the number of vcs commands.
  const size_t GetCommands() const {
    return m_Commands.size();};
  
  /// Gets the flags used to run the command.
  const wxString GetFlags() const;

  /// Gets the name for this vcs entry.
  const wxString& GetName() const {return m_Name;};

  /// Sets the current vcs command.
  /// Returns true if command was set.
  bool SetCommand(
    /// a command no from commands
    int command_no);

#if wxUSE_GUI
  /// Shows a dialog allowing you to run or cancel the current vcs command.
  /// Returns result from calling ShowModal.
  int ShowDialog(
    wxWindow* parent, 
    const wxString& caption,
    bool add_folder) const;
#endif

#if wxUSE_GUI
  /// Overriden from base class.
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif
private:
  enum
  {
    VCS_FLAGS_LOCATION_POSTFIX,
    VCS_FLAGS_LOCATION_PREFIX 
  };
  
  void AddCommands(const wxXmlNode* node);
  const wxString GetBin() const;

  // no const, as entry is set using operator+ in wxExVCS.
  bool m_AdminDirIsTopLevel;
  
  int m_CommandIndex;
  int m_FlagsLocation;
  
  wxString m_AdminDir;
  wxString m_FlagsKey;
  wxString m_Name;
  
  wxExLexer m_Lexer;

  std::vector<wxExVCSCommand> m_Commands;
};
#endif
