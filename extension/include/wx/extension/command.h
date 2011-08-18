////////////////////////////////////////////////////////////////////////////////
// Name:      command.h
// Purpose:   Declaration of wxExCommand class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXCOMMAND_H
#define _EXCOMMAND_H

#include <wx/extension/stcdlg.h>

/// This class offers functionality around wxExecute.
class WXDLLIMPEXP_BASE wxExCommand
{
public:
  /// Default constructor.
  wxExCommand();
  
  /// Executes the command.
  long Execute(const wxString& command, const wxString& wd = wxEmptyString);
  
  /// Returns true if the output contains error info instead of
  /// normal vcs info.
  bool GetError() const {return m_Error;};

  /// Gets the output from Execute.
  const wxString& GetOutput() const {return m_Output;};
  
#if wxUSE_GUI
  /// Shows output from Execute.
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif
protected:
  /// Gets the dialog used for presenting the output.
#if wxUSE_GUI
  static wxExSTCEntryDialog* GetDialog() {return m_Dialog;};
#endif
private:
  bool m_Error;
  wxString m_Command;
  wxString m_Output;
  
#if wxUSE_GUI
  static wxExSTCEntryDialog* m_Dialog;
#endif  
};

#endif
