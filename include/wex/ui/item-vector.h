////////////////////////////////////////////////////////////////////////////////
// Name:      item-vector.h
// Purpose:   Declaration of wex::item_vector class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/ui/item.h>

import<any>;
import<vector>;

namespace wex
{
/// Offers a class to add functionality to a vector of items.
class item_vector
{
public:
  /// Constructor, provide the vector of items.
  explicit item_vector(const std::vector<item>* v)
    : m_v(v)
  {
    ;
  };

  /// Finds item by label.
  /// First tries to get the item from the config,
  /// if not yet present, returns the default value from the vector.
  template <typename T> const T find(const std::string& label) const
  {
    if (config(label).exists())
    {
      return std::any_cast<T>(config(label).get(T()));
    }

    T value;

    try
    {
      if (!find<T>(m_v, label, value))
      {
        log("unknown item") << label;
      }
    }
    catch (std::bad_cast& e)
    {
      log(e) << label;
    }

    return value;
  }

private:
  /// Retrieves current value for label.
  template <typename T>
  bool
  find(const std::vector<item>* items, const std::string& label, T& value) const
  {
    for (const auto& item : *items)
    {
      if (item.label() == label)
      {
        value = std::any_cast<T>(item.get_value());
        return true;
      }
      else if (item.is_notebook())
      {
        for (const auto& page :
             std::any_cast<item::notebook_t>(item.data().initial()))
        {
          if (find<T>(&page.second, label, value))
          {
            return true;
          }
        }
      }
      else if (item.type() == item::CHECKLISTBOX_BOOL)
      {
        for (const auto& choice :
             std::any_cast<item::choices_bool_t>(item.data().initial()))
        {
          if (before(choice, ',') == label)
          {
            value = std::any_cast<T>(choice.find(",") != std::string::npos);
            return true;
          }
        }
      }
    }

    return false;
  }

  const std::vector<item>* m_v;
};
} // namespace wex
