////////////////////////////////////////////////////////////////////////////////
// Name:      support.h
// Purpose:   Declaration of decoratedframe class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma

#include <wex/report/frame.h>

class app;

class decorated_frame : public wex::history_frame
{
public:
  decorated_frame(app* app);
  virtual bool AllowClose(wxWindowID id, wxWindow* page) override;
  virtual void OnNotebook(wxWindowID id, wxWindow* page) override;
protected:
  app* m_App;
};
