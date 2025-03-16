////////////////////////////////////////////////////////////////////////////////
// Name:      reflection.h
// Purpose:   Declaration of wex::reflection class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <sstream>
#include <vector>

/// You can use this macro to add reflection to your class members,
/// or functions.
#define REFLECT_ADD(NAME, ITEM)                                                \
  {NAME,                                                                       \
   [&]()                                                                       \
   {                                                                           \
     return ITEM;                                                              \
   }}

namespace wex
{
/// This class offers reflection for any kind of class members,
/// as long as you add a conversion from / to std::any.
class reflection
{
public:
  /// Callback for functions.
  /// Just return your class member, as an std::any type.
  typedef std::function<std::any()> function_t;

  /// Determines what to log.
  enum class log_t
  {
    ALL,        ///< log empty values as well
    SKIP_EMPTY, ///< do not log empty values
  };

  /// The reflection type, as a struct of name and callback.
  typedef struct reflection_t
  {
    std::string name;
    function_t  f;
  } reflection_t;

  /// Default constructor, sets all reflection items and log_t.
  explicit reflection(
    const std::vector<reflection_t>& v = {},
    log_t                              = log_t::ALL);

  /// Logs all items to a std::stringstream.
  std::stringstream log() const;

private:
  std::vector<reflection_t> m_items;
  log_t                     m_log_t;
};
}; // namespace wex
