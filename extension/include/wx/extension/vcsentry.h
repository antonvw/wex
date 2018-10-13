////////////////////////////////////////////////////////////////////////////////
// Name:      vcs_entry.h
// Purpose:   Declaration of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <pugixml.hpp>
#include <wx/extension/lexer.h>
#include <wx/extension/menucommands.h>
#include <wx/extension/process.h>
#include <wx/extension/vcscommand.h>

namespace wex
{
  class menu;

  /// This class collects a single vcs.
  class vcs_entry : public process, public menu_commands < vcs_command >
  {
  public:
    enum
    {
      VCS_FLAGS_LOCATION_POSTFIX,
      VCS_FLAGS_LOCATION_PREFIX 
    };
    
    /// Default constructor.
    vcs_entry(
      /// name of the vcs
      const std::string& name = std::string(),
      /// which dir is used for vcs admin (like .svn, .git)
      const std::string& admin_dir = std::string(),
      /// commands for this vcs,
      /// default adds empty vcs command with id 0
      const std::vector<vcs_command> & commands = std::vector<vcs_command>{vcs_command()},
      /// vcs flags
      int flags_location = VCS_FLAGS_LOCATION_POSTFIX);
    
    /// Constructor using xml node.
    vcs_entry(const pugi::xml_node& node);

    /// Returns true if admin dir is only at top level.
    bool AdminDirIsTopLevel() const {return m_AdminDirIsTopLevel;};

    /// Builds a menu from all vcs commands.
    /// Returns (total) number of items in menu.
    int BuildMenu(
      /// menu id to be added to the vcs commands
      int base_id, 
      /// menu to be built
      menu* menu, 
      /// default assumes this is a popup menu
      bool is_popup = true) const;

    /// Executes the current vcs command (from SetCommand), or
    /// the first command if SetCommand was not yet invoked.
    /// Might ask for vcs binary if it is not yet known.
    /// Return code is code from process Execute,
    /// and also can be false if dialog for vcs bin was cancelled.
    bool Execute(
      /// args, like filenames, or vcs flags
      const std::string& args = std::string(),
      /// lexer that is used for presenting the output
      const lexer& lexer = wex::lexer(),
      /// wait to finish
      long flags = PROCESS_EXEC_WAIT,
      /// working directory
      const std::string& wd = std::string());
    
    /// Returns the administrative directory.
    const auto& GetAdminDir() const {return m_AdminDir;};

    /// Returns the name of current branch.
    const std::string GetBranch() const;

    /// Returns the flags used to run the command.
    const std::string GetFlags() const;

    /// Returns flags location.
    auto GetFlagsLocation() const {return m_FlagsLocation;};

    /// Returns margin size.
    auto GetMarginWidth() const {return m_MarginWidth;};

    /// Returns pos begin.
    const auto & GetPosBegin() const {return m_PosBegin;};

    /// Returns pos end.
    const auto & GetPosEnd() const {return m_PosEnd;};

    virtual void ShowOutput(const std::string& caption = std::string()) const override;
  private:
    // no const, as entry is set using operator+ in vcs.
    bool m_AdminDirIsTopLevel {false};
    int m_FlagsLocation, m_MarginWidth;
    std::string m_AdminDir, m_PosBegin, m_PosEnd;
    lexer m_Lexer;
  };
};
