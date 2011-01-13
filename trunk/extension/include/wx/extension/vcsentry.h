////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.h
// Purpose:   Declaration of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Created:   2010-08-27
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVCSENTRY_H
#define _EXVCSENTRY_H

#include <vector>
#include <wx/menu.h>
#include <wx/xml/xml.h>
#include <wx/extension/vcscommand.h>
#include <wx/extension/command.h>
#include <wx/extension/filename.h>

/// This class collects a single vcs.
class WXDLLIMPEXP_BASE wxExVCSEntry : public wxExCommand
{
public:
  enum wxExVCSEntryFlagsLocation
  {
    VCS_FLAGS_LOCATION_POSTFIX,
    VCS_FLAGS_LOCATION_PREFIX,
  };
  
  /// Default constructor.
  wxExVCSEntry();
  
  /// Constructor using xml node.
  wxExVCSEntry(const wxXmlNode* node);

#if wxUSE_GUI
  /// Builds a menu, default assumes it is a popup menu.
  /// Returns number of items in menu.
  int BuildMenu(int base_id, wxMenu* menu, bool is_popup = true) const;
#endif

  /// Executes the current command for this vcs.
  long Execute(
    const wxExFileName& filename,
    const wxString& args,
    const wxString& wd = wxEmptyString);
  
  /// Gets the current vcs command.  
  const wxExVCSCommand& GetCommand() const {
    return m_Commands.at(m_CommandId);};

  /// Gets the flags location.
  const int GetFlagsLocation() const {return m_FlagsLocation;};
  
  /// Gets the name.
  const wxString& GetName() const {return m_Name;};

  /// Gets the no.
  int GetNo() const {return m_No;};
  
  /// Resets the number of instances, so the no
  /// will start from begin again.
  static void ResetInstances();

  /// Sets the current vcs command.
  void SetCommand(int menu_id);

#if wxUSE_GUI
  /// Overriden from base class.
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif
  
  /// Does this vcs supports keyword expansion.
  bool SupportKeywordExpansion() const {return m_SupportKeywordExpansion;};
private:
  void AddCommands(const wxXmlNode* node);

  static int m_Instances;
  
  // no const, as entry is set using operator+ in wxExVCS.
  bool m_SupportKeywordExpansion;
  int m_CommandId;
  int m_FlagsLocation;
  int m_No;
  wxString m_Name;
  wxExFileName m_FileName;

  std::vector<wxExVCSCommand> m_Commands;
};
#endif
