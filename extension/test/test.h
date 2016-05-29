////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wx/extension/app.h>
#include <wx/extension/filename.h>

class wxExManagedFrame;

namespace Catch
{
  class Session;
}

/// Adds managed pane.
void AddPane(wxExManagedFrame* frame, wxWindow* pane);

/// Returns test dir.
const wxString GetTestDir();

/// Returns test file.
const wxExFileName GetTestFile();
  
/// Sets environment. 
void SetEnvironment(const wxString& dir);

/// Sets working directory to test dir, returns current working directory.
const wxString SetWorkingDirectory();

/// Invoke UI action on window, 
/// Returns false if an error occurred.
bool wxExUIAction(
  /// window to operate on
  wxWindow* win, 
  /// action, if starts with button is a button action,
  /// if starts with dialog is a dialog action
  const wxString& action = "button",
  /// e.g. the dialog to operate on
  const wxString& par = "right");
  
/// Derive your application from wxExApp.
class wxExTestApp: public wxExApp
{
public:
  /// Constructor.
  wxExTestApp() {}

  /// Cleanup.
  virtual int OnExit() override;
  
  /// Prepare environment.
  virtual bool OnInit() override;

  /// Start event loop and start testing.
  virtual int OnRun() override;

  /// Sets catch session.
  void SetSession(Catch::Session* session);
private:
  Catch::Session* m_Session;
};

int wxExTestMain(int argc, char* argv[], wxExTestApp* app, bool use_eventloop);
