////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex report sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/app.h>
#include <wex/notebook.h>
#include <wex/stc.h>
#include <wex/report/frame.h>

/// Derive your application from wex::app.
class app: public wex::app
{
public:
  app() {}
private:
  bool OnInit() override;
};

class frame: public wex::report::frame
{
public:
  frame();
protected:
  wex::report::listview* activate(
    wex::listview_data::type_t type, 
    const wex::lexer* lexer = nullptr) override;
  bool allow_close(wxWindowID id, wxWindow* page) override;
  wex::listview* get_listview() override;
  wex::stc* get_stc() override;
  wex::stc* open_file(
    const wex::path& file,
    const wex::stc_data& stc_data = wex::stc_data()) override;
private:
  wex::notebook* m_notebookWithLists; ///< all listviews
  wex::stc* m_stc;
};
