////////////////////////////////////////////////////////////////////////////////
// Name:      support.h
// Purpose:   Declaration of DecoratedFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma

#include <wx/extension/report/frame.h>

class DecoratedFrame : public wxExFrameWithHistory
{
public:
  DecoratedFrame();
  virtual bool AllowClose(wxWindowID id, wxWindow* page) override;
  virtual void OnNotebook(wxWindowID id, wxWindow* page) override;
};
