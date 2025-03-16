////////////////////////////////////////////////////////////////////////////////
// Name:      test-item-config.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ui/item.h>

#include "test-configitem.h"
#include "test.h"

TEST_CASE("wex::item-config")
{
  ITEM_START()

  SUBCASE("config")
  {
    wex::item::use_config(true);

    // Use specific constructors.
    const wex::item ci_empty;
    const wex::item ci_spacer(5);
    const wex::item ci_cb("ci-cb", wex::item::COMBOBOX);
    const wex::item ci_lb(
      "ci-lb",
      wex::item::LISTBOX,
      std::list<std::string>{"x:1", "y"});
    const wex::item ci_cb_dir("ci-cb-dir", wex::item::COMBOBOX_DIR);
    const wex::item ci_sp("ci-sp", 1, 5);
    const wex::item ci_sp_d(
      "ci-sp-d",
      1.0,
      5.0,
      1.0,
      wex::data::item().inc(static_cast<double>(1.0)));
    const wex::item ci_sl("ci-sl", 1, 5, 2, wex::item::SLIDER);
    const wex::item ci_vl(wxLI_HORIZONTAL);
    wex::item       ci_str("ci-string", std::string());
    const wex::item ci_hl(
      "ci-hyper",
      "www.wxwidgets.org",
      wex::item::HYPERLINKCTRL);
    wex::item       ci_st("ci-static", "HELLO", wex::item::STATICTEXT);
    const wex::item ci_int("ci-int", wex::item::TEXTCTRL_INT);
    const wex::item ci_grid("ci-grid", wex::item::GRID);
    const wex::item ci_rb("ci-rb", {{0, "Zero"}, {1, "One"}, {2, "Two"}}, true);
    const wex::item ci_bc(
      "ci-cl",
      {{0, "Bit One"}, {1, "Bit Two"}, {2, "Bit Three"}, {4, "Bit Four"}},
      false);
    const wex::item ci_cl_n({"This", "Or", "Other"});
    const wex::item ci_user(
      "ci-user",
      new wxTextCtrl(),
      wex::data::item()
        .user_window_create(
          [=](wxWindow* user, wxWindow* parent)
          {
            (reinterpret_cast<wxTextCtrl*>(user))->Create(parent, 100);
          })
        .user_window_to_config(
          [=](wxWindow* user, bool save)
          {
            if (save)
            {
              wex::config("mytext").set((reinterpret_cast<wxTextCtrl*>(user))
                                          ->GetValue()
                                          .ToStdString());
            }
            return true;
          })
        .apply(
          [=](wxWindow* user, const std::any& value, bool save)
          {
            wex::log::status(
              ((reinterpret_cast<wxTextCtrl*>(user))->GetValue()));
          }));

    REQUIRE(ci_empty.type() == wex::item::EMPTY);
    REQUIRE(ci_empty.empty());
    REQUIRE(!ci_empty.is_row_growable());
    REQUIRE(ci_cb.type() == wex::item::COMBOBOX);
    REQUIRE(ci_lb.type() == wex::item::LISTBOX);
    REQUIRE(ci_cb_dir.type() == wex::item::COMBOBOX_DIR);
    REQUIRE(ci_spacer.type() == wex::item::SPACER);
    REQUIRE(ci_grid.type() == wex::item::GRID);
    REQUIRE(ci_sl.label() == "ci-sl");
    REQUIRE(ci_sl.type() == wex::item::SLIDER);
    REQUIRE(ci_vl.type() == wex::item::STATICLINE);
    REQUIRE(ci_sp.label() == "ci-sp");
    REQUIRE(ci_sp.type() == wex::item::SPINCTRL);
    REQUIRE(ci_sp_d.type() == wex::item::SPINCTRLDOUBLE);
    REQUIRE(ci_str.type() == wex::item::TEXTCTRL);
    REQUIRE(ci_hl.type() == wex::item::HYPERLINKCTRL);
    REQUIRE(ci_st.type() == wex::item::STATICTEXT);
    REQUIRE(ci_int.type() == wex::item::TEXTCTRL_INT);
    REQUIRE(ci_rb.type() == wex::item::RADIOBOX);
    REQUIRE(ci_bc.type() == wex::item::CHECKLISTBOX_BIT);
    REQUIRE(ci_cl_n.type() == wex::item::CHECKLISTBOX_BOOL);
    REQUIRE(ci_user.type() == wex::item::USER);

    std::vector<wex::item> items{
      ci_empty,
      ci_spacer,
      ci_cb,
      ci_lb,
      ci_cb_dir,
      ci_sl,
      ci_vl,
      ci_sp,
      ci_sp_d,
      ci_str,
      ci_hl,
      ci_st,
      ci_int,
      ci_rb,
      ci_bc,
      ci_cl_n,
      ci_grid,
      ci_user};

    const auto& more(wex::test_config_item().vector(0, 1));
    items.insert(items.end(), more.begin(), more.end());

    // Check members are initialized.
    for (auto& it : items)
    {
      REQUIRE(it.data().columns() == 1);

      if (it.type() == wex::item::USER)
      {
        REQUIRE(it.window() != nullptr);
      }
      else
      {
        REQUIRE(it.window() == nullptr);
      }

      if (
        it.type() != wex::item::STATICLINE && it.type() != wex::item::SPACER &&
        it.type() != wex::item::EMPTY)
      {
        REQUIRE(!it.label().empty());
        REQUIRE(!it.empty());
      }

      it.set_row_growable(true);
    }

    // layout the items and check control is created.
    for (auto& it : items)
    {
      // Testing on not nullptr not possible,
      // not all items need a sizer.
      it.layout(layout);

      if (it.type() != wex::item::EMPTY && it.type() != wex::item::SPACER)
      {
        CAPTURE(it.label());
        CAPTURE(it.type());

        REQUIRE(it.window() != nullptr);

        if (
          it.type() == wex::item::CHECKLISTBOX_BOOL ||
          it.type() == wex::item::RADIOBOX ||
          it.type() == wex::item::STATICLINE || it.type() == wex::item::USER ||
          it.type() == wex::item::STATICTEXT || it.is_notebook())
        {
          REQUIRE(!it.get_value().has_value());
        }
        else
        {
          REQUIRE(it.get_value().has_value());
        }
      }
    }

    REQUIRE(ci_user.data().apply());

    // Now check to_config (after layout).
    // These are copies, the window() is nullptr!
    REQUIRE(ci_str.layout(layout) != nullptr);
    REQUIRE(ci_st.layout(layout) != nullptr);
    REQUIRE(ci_str.to_config(true));
    REQUIRE(ci_str.to_config(false));
    REQUIRE(!ci_st.to_config(true));
    REQUIRE(!ci_st.to_config(false));
    REQUIRE(ci_user.to_config(true));
    REQUIRE(ci_user.to_config(false));
    REQUIRE(!ci_grid.to_config(false));
  }
}
