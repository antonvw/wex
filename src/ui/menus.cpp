////////////////////////////////////////////////////////////////////////////////
// Name:      menus.cpp
// Purpose:   Implementation of wex::menus class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
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

bool wex::menus::allow_add_menu(const menu_command& mc, const menu* menu)
{
  bool add = false;

  if (
    mc.type().test(menu_command::IS_POPUP) &&
    mc.type().test(menu_command::IS_MAIN))
  {
    add = true;
  }
  else if (mc.type().test(menu_command::IS_POPUP))
  {
    add = menu->style().test(menu::IS_POPUP);
  }
  else if (mc.type().test(menu_command::IS_MAIN))
  {
    add = !menu->style().test(menu::IS_POPUP);
  }

  if (
    (menu->style().test(menu::IS_SELECTED) &&
     !mc.type().test(menu_command::IS_SELECTED)) ||
    (!menu->style().test(menu::IS_SELECTED) &&
     mc.type().test(menu_command::IS_SELECTED)))
  {
    add = false;
  }

  if (
    !menu->style().test(menu::IS_LINES) &&
    mc.type().test(menu_command::IS_LINES))
  {
    add = false;
  }

  if (
    !menu->style().test(menu::IS_VISUAL) &&
    mc.type().test(menu_command::IS_VISUAL))
  {
    add = false;
  }

  return add;
}

void wex::menus::no_commands_added(const pugi::xml_node& node)
{
  const std::string name(node.attribute("name").value());

  if (!name.empty())
  {
    log("no commands found for") << name;
  }
}

const wex::path wex::menus::path()
{
  return wex::path(config::dir(), "wex-menus.xml");
}
