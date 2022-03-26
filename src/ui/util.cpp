////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wex::ui utils
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include "item.h"

namespace wex
{
const std::any get_value_prim(const wex::item* item)
{
  switch (item->type())
  {
    case item::CHECKBOX:
      if (item->data().initial().has_value())
      {
        return std::any_cast<bool>(item->data().initial());
      }
      else
      {
        return false;
      }

    case item::CHECKLISTBOX_BIT:
    case item::RADIOBOX:
    {
      long value = 0;
      for (const auto& b :
           std::any_cast<wex::item::choices_t>(item->data().initial()))
      {
        if (b.second.find(",") != std::string::npos)
        {
          value |= b.first;
        }
      }

      return std::any(value);
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

const std::string str(const std::string& name, const std::any& any)
{
  std::stringstream s;

  s << name << "{internal type: " << any.type().name() << ", value: ";

  if (any.has_value())
  {
    try
    {
      if (any.type() == typeid(int))
      {
        s << std::any_cast<int>(any);
      }
      else if (any.type() == typeid(long))
      {
        s << std::any_cast<long>(any);
      }
      else if (any.type() == typeid(double))
      {
        s << std::any_cast<double>(any);
      }
      else if (any.type() == typeid(std::string))
      {
        s << std::any_cast<std::string>(any);
      }
      else
      {
        s << "<no cast available>";
      }
    }
    catch (std::bad_cast& e)
    {
      s << "<log bad cast: " << e.what() << ">";
    }
  }
  else
  {
    s << "<no value>";
  }

  s << "} ";

  return s.str();
}

const item::type_t use_type(const std::string& label, item::type_t t)
{
  return label.find(':') != std::string::npos ? item::STATICTEXT : t;
}
} // namespace wex
