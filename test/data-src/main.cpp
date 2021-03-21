////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

namespace wex::test
{
  class data : public app
  {
  public:
    static auto* get_listview() { return m_listview; };
    static auto* get_stc() { return m_stc; };

  private:
    bool OnInit() override;

    wxFrame* m_frame{nullptr};

    static inline wex::factory::listview* m_listview = nullptr;
    static inline wex::factory::stc*      m_stc      = nullptr;
  };
}; // namespace wex::test

class data_listview : public wex::factory::listview
{
public:
  data_listview(wxFrame* parent)
  {
    Create(parent, -1);
    Show();
  }
};

class data_stc : public wex::factory::stc
{
public:
  data_stc(wxFrame* parent)
  {
    Create(parent, -1);
    Show();
  };

private:
  const wex::path& get_filename() const override { return m_path; };
  wex::path        m_path;

  static inline wxFrame* m_frame = nullptr;
};

IMPLEMENT_APP_NO_MAIN(wex::test::data);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::data());
}

bool wex::test::data::OnInit()
{
  wex::test::app::OnInit();

  m_frame = new wxFrame(nullptr, wxID_ANY, "test");
  m_frame->Show();

  m_listview = new data_listview(m_frame);
  m_stc      = new data_stc(m_frame);

  return true;
}

wex::factory::listview* get_listview()
{
  return wex::test::data::get_listview();
}

wex::factory::stc* get_stc()
{
  return wex::test::data::get_stc();
}
