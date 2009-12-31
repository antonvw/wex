////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Test file for running cppunit testing
// Author:    Anton van Wezenbeek
// Created:   2009-07-07
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

// The actual contents do not matter. This file is not compiled.
#include <wx/extension/extension.h>

/// Derive your application from wxExApp.
class wxExTestApp: public wxExApp
{
public:
  /// Constructor.
  wxExTestApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
};
