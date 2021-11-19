////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/item.h>
#include <wx/artprov.h>
#include <wx/imaglist.h>

#include "test-configitem.h"
#include "test-item.h"
#include "test.h"

TEST_CASE("wex::item")
{
  auto* panel = new wxScrolledWindow(frame());
  frame()->pane_add(panel);
  auto* sizer = new wxFlexGridSizer(4);
  panel->SetSizer(sizer);
  panel->SetScrollbars(20, 20, 50, 50);

  SUBCASE("no config")
  {
    wex::item::use_config(false);

    wex::item item(
      "item",
      "hello string",
      wex::item::TEXTCTRL,
      wex::data::item(wex::data::control().is_required(true)));

    REQUIRE(item.data().columns() == 1);
    REQUIRE(
      std::any_cast<std::string>(item.data().initial()) == "hello string");
    REQUIRE(item.data().control().is_required());
    REQUIRE(item.label() == "item");
    REQUIRE(item.page().empty());
    REQUIRE(item.type() == wex::item::TEXTCTRL);
    REQUIRE(item.window() == nullptr);
    REQUIRE(item.get_value().has_value());
    REQUIRE(!item.is_row_growable());
    REQUIRE(!item.data().apply());

    REQUIRE(!item.to_config(false));
    wex::item::use_config(true);
    REQUIRE(!item.to_config(false));
    wex::item::use_config(false);

    item.set_dialog(nullptr);

    // setting value if window is nullptr should have no effect.
    REQUIRE(!item.set_value("test"));
    REQUIRE(item.get_value().has_value());

    REQUIRE(item.layout(panel, sizer) != nullptr);
    REQUIRE(item.window() != nullptr);
    REQUIRE(std::any_cast<std::string>(item.get_value()) == "hello string");
    REQUIRE(item.set_value(std::string("value changed")));
    REQUIRE(std::any_cast<std::string>(item.get_value()) == "value changed");
    REQUIRE(
      std::any_cast<std::string>(item.data().initial()) == "hello string");

    item.set_row_growable(true);
    REQUIRE(item.is_row_growable());

    wex::item item_int("int", wex::item::TEXTCTRL_INT, std::string("100"));

    REQUIRE(item_int.type() == wex::item::TEXTCTRL_INT);
    REQUIRE(item_int.layout(panel, sizer) != nullptr);
    REQUIRE(item_int.window() != nullptr);
    REQUIRE(std::any_cast<long>(item_int.get_value()) == 100);
    REQUIRE(std::any_cast<std::string>(item_int.data().initial()) == "100");
    REQUIRE(item_int.set_value(300l));
    REQUIRE(std::any_cast<long>(item_int.get_value()) == 300);
    REQUIRE(std::any_cast<std::string>(item_int.data().initial()) == "100");

    wex::item item_int2("int", wex::item::TEXTCTRL_INT, std::string("xxx"));
    REQUIRE(item_int2.type() == wex::item::TEXTCTRL_INT);
    REQUIRE(item_int2.layout(panel, sizer) != nullptr);
    REQUIRE(item_int2.window() != nullptr);

    try
    {
      // an excption should be raised as xxx cannot be converted to
      // a long.
      auto val = std::any_cast<long>(item_int2.get_value());
      // therefore, we should not come here
      REQUIRE(1 == 0);
    }
    catch (std::exception&)
    {
    }

    wex::item item_float(
      "float",
      wex::item::TEXTCTRL_FLOAT,
      std::string("100.001"));
    REQUIRE(item_float.type() == wex::item::TEXTCTRL_FLOAT);
    item_float.layout(panel, sizer);
    REQUIRE(std::any_cast<double>(item_float.get_value()) == 100.001);

    wex::item
      item_spin("spindouble", 20.0, 30.0, 25.0, wex::data::item().inc(0.1));
    REQUIRE(item_spin.type() == wex::item::SPINCTRLDOUBLE);

#ifdef __UNIX__
    wex::item item_picker(
      "picker",
      wex::item::FILEPICKERCTRL,
      std::string("/usr/bin/git"));
#endif

#ifdef __UNIX__
    REQUIRE(item_picker.layout(panel, sizer) != nullptr);
    REQUIRE(
      std::any_cast<std::string>(item_picker.get_value()) == "/usr/bin/git");
#endif

    std::vector<wex::item> items{
      item,
      item_int,
      item_spin
#ifdef __UNIX__
      ,
      item_picker
#endif
    };

    const auto more(test_items());
    items.insert(items.end(), more.begin(), more.end());

    // layout the items and check control is created.
    for (auto& it : items)
    {
      // wex::item::USER is not yet laid out ok, gives errors.
      if (it.type() != wex::item::USER)
      {
        // Testing on not nullptr not possible,
        // not all items need a sizer.
        it.layout(panel, sizer);
      }

      if (it.type() != wex::item::EMPTY && it.type() != wex::item::SPACER)
      {
        REQUIRE(it.window() != nullptr);
      }
    }
  }

  SUBCASE("config")
  {
    wex::item::use_config(true);

    // Use specific constructors.
    const wex::item ci_empty;
    const wex::item ci_spacer(5);
    const wex::item ci_cb("ci-cb", wex::item::COMBOBOX);
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
      "ci-usr",
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
              wex::config("mytext").set((reinterpret_cast<wxTextCtrl*>(user))
                                          ->GetValue()
                                          .ToStdString());
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

    const auto more(test_config_items(0, 1));
    items.insert(items.end(), more.begin(), more.end());

    // Check members are initialized.
    for (auto& it : items)
    {
      REQUIRE(it.data().columns() == 1);

      if (it.type() == wex::item::USER)
        REQUIRE(it.window() != nullptr);
      else
        REQUIRE(it.window() == nullptr);

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
      it.layout(panel, sizer);

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
    REQUIRE(ci_str.layout(panel, sizer) != nullptr);
    REQUIRE(ci_st.layout(panel, sizer) != nullptr);
    REQUIRE(ci_str.to_config(true));
    REQUIRE(ci_str.to_config(false));
    REQUIRE(!ci_st.to_config(true));
    REQUIRE(!ci_st.to_config(false));
    REQUIRE(ci_user.to_config(true));
    REQUIRE(ci_user.to_config(false));
    REQUIRE(!ci_grid.to_config(false));
  }

  SUBCASE("label")
  {
    wex::item::use_config(true);
    wex::item item("item-parent.child", "karmeliet");

    REQUIRE(item.layout(panel, sizer) != nullptr);
    REQUIRE(item.label() == "item-parent.child");
    REQUIRE(std::any_cast<std::string>(item.get_value()) == "karmeliet");
    REQUIRE(item.to_config(true));
    REQUIRE(wex::config("item-parent").exists());
    REQUIRE(!wex::config("item-parent").is_child());
    REQUIRE(wex::config("item-parent.child").exists());
    REQUIRE(!wex::config("item-parent.child").is_child());
    REQUIRE(wex::config("item-parent.child").get() == "karmeliet");
  }

  SUBCASE("group")
  {
    auto* dlg = new wex::item_dialog(
      {{{"group", {{"element1"}, {"element2"}, {"element3"}, {"element4"}}}}},
      wex::data::window().button(wxOK | wxCANCEL | wxAPPLY));

    dlg->Show();
  }

  SUBCASE("notebooks")
  {
    const std::vector<std::string> titles{
      "item::NOTEBOOK",
      "item::NOTEBOOK_AUI",
      "item::NOTEBOOK_CHOICE",
      "item::NOTEBOOK_LIST",
      "item::NOTEBOOK_SIMPLE",
      "item::NOTEBOOK_TOOL",
      "item::NOTEBOOK_TREE",
      "item::NOTEBOOK_WEX"};

    REQUIRE(titles.size() == wex::item::NOTEBOOK_WEX - wex::item::NOTEBOOK + 1);

    // Test dialog using notebook with pages.
    for (int style = wex::item::NOTEBOOK; style <= wex::item::NOTEBOOK_WEX;
         style++)
    {
      CAPTURE(titles[style - wex::item::NOTEBOOK]);

      wxImageList* il = nullptr;

      if (style == wex::item::NOTEBOOK_TOOL)
      {
        const wxSize imageSize(32, 32);

        il = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());

        il->Add(
          wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));
      }

      auto* dlg = new wex::item_dialog(
        {test_notebook_item(
          (wex::item::type_t)style,
          wex::data::item::LABEL_NONE,
          il)},
        wex::data::window()
          .button(wxOK | wxCANCEL | wxAPPLY)
          .title(titles[style - wex::item::NOTEBOOK]));

      dlg->Show();

      REQUIRE(
        std::any_cast<std::string>(dlg->find("string1").data().initial()) ==
        "first");
      REQUIRE(
        std::any_cast<std::string>(dlg->find("string1").get_value()) ==
        "first");
      REQUIRE(dlg->set_item_value("string1", std::string("xxx")));
      REQUIRE(
        std::any_cast<std::string>(dlg->find("string1").get_value()) == "xxx");

      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
    }
  }

  SUBCASE("validate")
  {
    wex::item item(
      "item",
      "testxxx",
      wex::item::TEXTCTRL,
      wex::data::item(wex::data::control().is_required(true)));

    REQUIRE(item.validate("[a-z]+"));
    REQUIRE(!item.validate("[0-9]+"));
  }
}
