////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025
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
  static inline wex::syntax::stc*       m_stc      = nullptr;
};
}; // namespace wex::test

IMPLEMENT_APP_NO_MAIN(wex::test::app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::common());
}

class common_stc : public wex::syntax::stc
{
public:
  explicit common_stc(wxFrame* parent) { Show(); };

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;
};

bool wex::test::common::OnInit()
{
  if (!wex::app::OnInit() || !on_init(this))
  {
    return false;
  }

  m_frame = new wex::factory::frame(nullptr, -1, "common test");
  m_frame->Show();

  m_listview = new factory::listview(data::window().parent(m_frame));
  m_stc      = new common_stc(m_frame);
  m_listview->Show();

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

wex::syntax::stc* get_stc()
{
  return wex::test::common::get_stc();
}

TEST_CASE("wex::test::app")
{
  wxGetApp().get_locale().GetName();
}
