////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.h
// Purpose:   Declaration of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <pugixml.hpp>
#include <wx/extension/lexer.h>
#include <wx/extension/menucommands.h>
#include <wx/extension/process.h>
#include <wx/extension/vcscommand.h>

class wxExItemDialog;
class wxExMenu;

/// This class collects a single vcs.
class WXDLLIMPEXP_BASE wxExVCSEntry : 
  public wxExProcess, 
  public wxExMenuCommands < wxExVCSCommand >
{
public:
  enum
  {
    VCS_FLAGS_LOCATION_POSTFIX,
    VCS_FLAGS_LOCATION_PREFIX 
  };
  
  /// Default constructor.
  wxExVCSEntry(
    /// name of the vcs
    const std::string& name = std::string(),
    /// which dir is used for vcs admin (like .svn, .git)
    const std::string& admin_dir = std::string(),
    /// commands for this vcs,
    /// default adds empty vcs command with id 0
    const std::vector<wxExVCSCommand> & commands = std::vector<wxExVCSCommand>{wxExVCSCommand()},
    /// vcs flags
    int flags_location = VCS_FLAGS_LOCATION_POSTFIX);
  
  /// Constructor using xml node.
  wxExVCSEntry(const pugi::xml_node& node);

  /// Returns true if admin dir is only at top level.
  bool AdminDirIsTopLevel() const {return m_AdminDirIsTopLevel;};

#if wxUSE_GUI
  /// Builds a menu from all vcs commands.
  /// Returns (total) number of items in menu.
  int BuildMenu(
    /// menu id to be added to the vcs commands
    int base_id, 
    /// menu to be built
    wxExMenu* menu, 
    /// default assumes this is a popup menu
    bool is_popup = true) const;
#endif

  /// Executes the current vcs command (from SetCommand), or
  /// the first command if SetCommand was not yet invoked.
  /// Might ask for vcs binary if it is not yet known.
  /// Return code is code from process Execute,
  /// and also can be false if dialog for vcs bin was cancelled.
  bool Execute(
    /// args, like filenames, or vcs flags
    const std::string& args = std::string(),
    /// lexer that is used for presenting the output
    const wxExLexer& lexer = wxExLexer(),
    /// wait to finish
    bool wait = true,    
    /// working directory
    const std::string& wd = std::string());
  
  /// Returns the administrative directory.
  const auto& GetAdminDir() const {return m_AdminDir;};

  /// Returns the flags used to run the command.
  const std::string GetFlags() const;

#if wxUSE_GUI
  /// Shows a dialog allowing you to run or cancel the current vcs command.
  /// Returns result from calling ShowModal.
  int ShowDialog(
    const wxExWindowData& data = wxExWindowData(),
    bool add_folder = false) const;
#endif

#if wxUSE_GUI
  virtual void ShowOutput(const std::string& caption = std::string()) const override;
#endif
private:
  // no const, as entry is set using operator+ in wxExVCS.
  bool m_AdminDirIsTopLevel = false;
  int m_FlagsLocation;
  
  std::string m_AdminDir;
  wxExLexer m_Lexer;

  static wxExItemDialog* m_ItemDialog;
};
