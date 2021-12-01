////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

namespace wex::test
{
class common : public app
{
public:
  static auto* get_frame() { return m_frame; }
  static auto* get_listview() { return m_listview; }
  static auto* get_stc() { return m_stc; }

private:
  bool OnInit() override;

  static inline wex::factory::frame*    m_frame    = {nullptr};
  static inline wex::factory::listview* m_listview = nullptr;
  static inline wex::factory::stc*      m_stc      = nullptr;
};
}; // namespace wex::test

IMPLEMENT_APP_NO_MAIN(wex::test::app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::common());
}

class common_frame : public wex::factory::frame
{
public:
  common_frame()
  {
    Create(nullptr, -1, "common");
    Show();
  }
};

class common_listview : public wex::factory::listview
{
public:
  explicit common_listview(wxFrame* parent)
  {
    Create(parent, -1);
    Show();
  }
};

class common_stc : public wex::factory::stc
{
public:
  explicit common_stc(wxFrame* parent)
  {
    Create(parent, -1);
    Show();
  };

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;
};

bool wex::test::common::OnInit()
{
  wex::test::app::OnInit();

  m_frame = new common_frame;
  m_frame->Show();

  m_listview = new common_listview(m_frame);
  m_stc      = new common_stc(m_frame);

  return true;
}

wex::factory::frame* get_frame()
{
  return wex::test::common::get_frame();
}

wex::factory::listview* get_listview()
{
  return wex::test::common::get_listview();
}

wex::factory::stc* get_stc()
{
  return wex::test::common::get_stc();
}

TEST_CASE("wex::test::app")
{
  wxGetApp().get_catalog_dir();
}
