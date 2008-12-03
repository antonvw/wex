/******************************************************************************\
* File:          process.h
* Purpose:       Declaration of class 'ftProcess'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _FTPROCESS_H
#define _FTPROCESS_H

#include <wx/process.h>

class ftListView;
class ftThread;

/// Offers a wxProcess with output to a listview.
class ftProcess : public wxProcess
{
public:
  /// Constructor.
  ftProcess(ftListView* listview, const wxString& command = wxEmptyString);

  // Checks whether input is available from process and updates the listview.
  // You should call it regularly.
  // This is done by the internal thread, so not for doxygen.
  void CheckInput();

  /// Shows a config dialog, returns dialog return code.
  static int ConfigDialog();

  /// Is the process running.
  bool IsRunning() const;

  /// Returns whether a process has been selected.
  static bool IsSelected() {
    return !m_Command.empty();};

  /// Runs the process asynchronously (this call immediately returns).
  /// The process output is collected in a separate thread and added to the listview.
  bool Run();

  /// Stops the process.
  void Stop();
private:
  virtual void OnTerminate(int pid, int status); // overriden

  static wxString m_Command;
  ftListView* m_Owner;
  ftThread* m_Thread;
};
#endif
