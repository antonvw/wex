////////////////////////////////////////////////////////////////////////////////
// Name:      test-dialog.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/dialog.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testDialog()
{
  wxExDialog(m_Frame, "hello").Show();
}
