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

class wxMenu;

/// This class collects a single vcs.
class WXDLLIMPEXP_BASE wxExVCSEntry
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
  
  /// Gets the command.
  const wxExVCSCommand GetCommand(int menu_id) const;

  /// Gets the flags location.
  const int GetFlagsLocation() const {return m_FlagsLocation;};
  
  /// Gets the name.
  const wxString& GetName() const {return m_Name;};

  /// Gets the no.
  long GetNo() const {return m_No;};
  
  /// Resets the number of instances, so the no
  /// will start from begin again.
  static void ResetInstances();

  /// Does this vcs supports keyword expansion.
  bool SupportKeywordExpansion() const {return m_SupportKeywordExpansion;};
private:
  void AddCommands(const wxXmlNode* node);

  static int m_Instances;

  const int m_FlagsLocation;
  const wxString m_Name;
  const long m_No;
  const bool m_SupportKeywordExpansion;

  std::vector<wxExVCSCommand> m_Commands;
};
#endif
