////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wex::ui utils
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>
#include <sstream>

#include "item.h"

namespace wex
{
const std::any get_value_prim(const wex::item* item)
{
  switch (item->type())
  {
    case item::CHECKBOX:
      return item->data().initial().has_value() ?
               std::any_cast<bool>(item->data().initial()) :
               false;

    case item::CHECKLISTBOX_BIT:
    case item::RADIOBOX:
    {
      const auto& choices(
        std::any_cast<wex::item::choices_t>(item->data().initial()));
      return std::any(std::accumulate(
        choices.begin(),
        choices.end(),
        0L,
        [](long a, const auto& b)
        {
          return (b.second.contains(",")) ? a |= b.first : a;
        }));
    }

    default:
      return item->data().initial();
  }
}

bool get_value_simple(wex::item::type_t t, wxWindow* window, std::any& any)
{
  switch (t)
  {
    case item::CHECKBOX:
      any = get_value_simple<wxCheckBox*>(window);
      break;

    case item::SLIDER:
      any = get_value_simple<wxSlider*>(window);
      break;

    case item::SPINCTRL:
      any = get_value_simple<wxSpinCtrl*>(window);
      break;

    case item::SPINCTRLDOUBLE:
      any = get_value_simple<wxSpinCtrlDouble*>(window);
      break;

    case item::TOGGLEBUTTON:
      any = get_value_simple<wxToggleButton*>(window);
      break;

    default:
      return false;
  }

  return true;
}

bool no_value(wex::item::type_t t)
{
  switch (t)
  {
    case item::BUTTON:
    case item::COMMANDLINKBUTTON:
    case item::GROUP:
    case item::STATICBOX:
    case item::STATICLINE:
    case item::STATICTEXT:
      break;

    case item::CHECKLISTBOX_BOOL:
    case item::RADIOBOX:
    case item::USER:
      // Not yet implemented
      break;

    default:
      return false;
  }

  return true;
}

const item::type_t use_type(const std::string& label, item::type_t t)
{
  return label.contains(':') ? item::STATICTEXT : t;
}
} // namespace wex
