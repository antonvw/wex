////////////////////////////////////////////////////////////////////////////////
// Name:      support.h
// Purpose:   Declaration of DecoratedFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <wx/extension/report/frame.h>

class DecoratedFrame : public wxExFrameWithHistory
{
public:
  DecoratedFrame();
  virtual bool AllowClose(wxWindowID id, wxWindow* page);
  virtual void OnNotebook(wxWindowID id, wxWindow* page);
protected:
  virtual void DoAddControl(wxExToolBar* bar);
};
#endif
