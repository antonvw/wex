////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex del sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>

/// Derive your application from wex::app.
class app : public wex::app
{
private:
  bool OnInit() override;
};

class frame : public wex::del::frame
{
public:
  frame();

private:
  wex::del::listview* activate(
    wex::data::listview::type_t type,
    const wex::lexer*           lexer = nullptr) override;
  bool           allow_close(wxWindowID id, wxWindow* page) override;
  wex::listview* get_listview() override;
  wex::stc*      get_stc() override;
  wex::stc*      open_file(
         const wex::path&      file,
         const wex::data::stc& data = wex::data::stc()) override;

  wex::notebook* m_notebook;
  wex::stc*      m_stc;
};
