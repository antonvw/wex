////////////////////////////////////////////////////////////////////////////////
// Name:      support.h
// Purpose:   Declaration of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma

#include <wex/report/frame.h>

class app;

class decorated_frame : public wex::report::frame
{
public:
  decorated_frame(app* app);
  bool allow_close(wxWindowID id, wxWindow* page) override;
  void on_notebook(wxWindowID id, wxWindow* page) override;
protected:
  app* m_app;
};
