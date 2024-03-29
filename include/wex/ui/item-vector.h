////////////////////////////////////////////////////////////////////////////////
// Name:      item-vector.h
// Purpose:   Declaration of wex::item_vector class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/ui/item.h>

#include <vector>

namespace wex
{
/// Offers a class to add functionality to a vector of items.
class item_vector
{
public:
  /// Constructor, provide the vector of items.
  explicit item_vector(const std::vector<item>* v);

  /// Finds item by label.
  /// First tries to get the item from the config,
  /// if not yet present, returns the default value from the vector.
  template <typename T> const T find(const std::string& label) const;

private:
  /// Retrieves current value for label.
  template <typename T>
  bool find(const std::vector<item>* items, const std::string& label, T& value)
    const;
  void log_error(const std::string& text) const;
  void log_error(const std::string& text, const std::exception& e) const;

  const std::vector<item>* m_v;
};

// implementation

template <typename T>
const T wex::item_vector::find(const std::string& label) const
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
      log_error(label);
    }
  }
  catch (std::bad_cast& e)
  {
    log_error(label, e);
  }

  return value;
}

template <typename T>
bool wex::item_vector::find(
  const std::vector<item>* items,
  const std::string&       label,
  T&                       value) const
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
        if (find_before(choice, ",") == label)
        {
          value = std::any_cast<T>(choice.contains(","));
          return true;
        }
      }
    }
  }

  return false;
}
} // namespace wex
