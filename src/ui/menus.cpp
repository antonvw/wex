////////////////////////////////////////////////////////////////////////////////
// Name:      menus.cpp
// Purpose:   Implementation of wex::menus class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/menus.h>

void wex::menus::add_menu(const menu_command& mc, menu* menu)
{
  const std::string  unused    = "XXXXX";
  static wex::menu*  submenu   = nullptr;
  static std::string prev_menu = unused;

  if (!mc.submenu().empty() && prev_menu != mc.submenu())
  {
    submenu   = new wex::menu();
    prev_menu = mc.submenu();
    menu->append({{}, {submenu, mc.submenu()}});
  }
  else if (mc.submenu().empty())
  {
    if (prev_menu != unused)
    {
      prev_menu = unused;
      menu->append({{}});
    }

    submenu = nullptr;
  }

  auto* usemenu = (submenu == nullptr ? menu : submenu);

  usemenu->append(
    {{m_id,
      ellipsed(
        mc.text().empty() ? mc.get_command(menu_command::INCLUDE_ACCELL) :
                            mc.text(),
        mc.control(),
        mc.type().test(menu_command::ELLIPSES))}});

  if (mc.type().test(menu_command::SEPARATOR))
  {
    usemenu->append({{}});
  }
}
