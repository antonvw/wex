////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.h
// Purpose:   Declaration of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVCSENTRY_H
#define _EXVCSENTRY_H

#include <vector>
#include <wx/menu.h>
#include <wx/xml/xml.h>
#include <wx/extension/command.h>
#include <wx/extension/lexer.h>
#include <wx/extension/vcscommand.h>

/// This class collects a single vcs.
class WXDLLIMPEXP_BASE wxExVCSEntry : public wxExCommand
{
public:
  /// Default constructor.
  wxExVCSEntry();
  
  /// Constructor using xml node.
  wxExVCSEntry(const wxXmlNode* node, int no);

#if wxUSE_GUI
  /// Builds a menu, default assumes it is a popup menu.
  /// Returns number of items in menu.
  int BuildMenu(int base_id, wxMenu* menu, bool is_popup = true) const;
#endif

  /// Executes the current command for this vcs.
  long Execute(
    const wxString& args,
    const wxExLexer& lexer,
    const wxString& wd = wxEmptyString);
  
  /// Gets the current vcs command.  
  const wxExVCSCommand& GetCommand() const {
    return m_Commands.at(m_CommandId);};

  /// Gets the flags used to run the command.
  const wxString GetFlags() const;

  /// Gets the name.
  const wxString& GetName() const {return m_Name;};

  /// Gets the no.
  int GetNo() const {return m_No;};
  
  /// Sets the current vcs command.
  void SetCommand(int menu_id);

#if wxUSE_GUI
  /// Shows a dialog allowing you to run or cancel the selected vcs command.
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
  
  /// Does this vcs support keyword expansion.
  bool SupportKeywordExpansion() const {return m_SupportKeywordExpansion;};
private:
  enum
  {
    VCS_FLAGS_LOCATION_POSTFIX,
    VCS_FLAGS_LOCATION_PREFIX,
  };
  
  void AddCommands(const wxXmlNode* node);
  const wxString GetBin() const;

  // no const, as entry is set using operator+ in wxExVCS.
  bool m_SupportKeywordExpansion;
  int m_CommandId;
  int m_FlagsLocation;
  int m_No;
  
  wxString m_FlagsKey;
  wxString m_Name;
  
  wxExLexer m_Lexer;

  std::vector<wxExVCSCommand> m_Commands;
};
#endif
