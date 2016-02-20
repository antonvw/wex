////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/extension/app.h>
#include <wx/extension/filename.h>

class wxExManagedFrame;

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

/// Invoke test function on window, wait for max time, then continue.
/// Returns false if an error occurred.
bool TestAndContinue(wxWindow* win, std::function<void(wxWindow* win)> f);
  
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
};
